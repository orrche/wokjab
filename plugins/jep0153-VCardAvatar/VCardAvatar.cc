/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: VCardAvatar
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Aug 28 23:42:20 2006
//

#include <sys/stat.h>
#include <sys/types.h>
#include "VCardAvatar.h"
#include <fstream>
#include <sstream>

#include "openssl/sha.h"
const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


VCardAvatar::VCardAvatar(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Avatar SetMy", &VCardAvatar::SetMy, 50);
	EXP_SIGHOOK("Jabber XML Presence", &VCardAvatar::Presence, 50);
	EXP_SIGHOOK("Jabber GUI GetIcon", &VCardAvatar::GetIcon, 50);
	EXP_SIGHOOK("Jabber XML Presence Send", &VCardAvatar::SendPresence, 50);
	mkdir((std::string(std::getenv("HOME")) + "/.wokjab").c_str(), 0700);
	mkdir((std::string(std::getenv("HOME")) + "/.wokjab/avatar").c_str(), 0700);
	
	ready = false;
}


VCardAvatar::~VCardAvatar()
{

}

std::string
VCardAvatar::Base64encode(const unsigned char *buf, int len)
{
	std::string ret("");
	int n = 0;
	
	for ( int i = (2.99 + len)/3 ; i ; i-- )
	{
		int data=0;
		
		if( len - n*3 == 1 )
			data = buf[n*3]*256*256;
		else if ( len - n*3 == 2 )
			data = buf[n*3+1]*256 + buf[n*3]*256*256;
		else
			data = buf[n*3+2] + buf[n*3+1]*256 + buf[n*3]*256*256;

		if( n * 3 > len )
		{
			ret += "="; 
			ret += "=";
		}
		else
		{
			ret += base64char[(data/64/64/64)%64];
			ret += base64char[(data/64/64)%64];
		}
		if( n * 3 + 2 > len )
			ret += "=";
		else
		{
			ret += base64char[(data/64)%64];
		}
		if( n * 3 + 3 > len )
			ret += "=";
		else
		{
			ret += base64char[data%64];
		}
		
		n++;
	}
	
	return ret;
}


int
VCardAvatar::SetMy(WokXMLTag *tag)
{
	std::string filename = tag->GetAttr("file");
	std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
	if ( !file.is_open() )
		return 1;
	int len = file.tellg();
	file.seekg (0, std::ios::beg);
	unsigned char *str = new unsigned char [len];
	file.read((char*)str, len);
	file.close();
	
	unsigned char buf[30];
	SHA1((unsigned char*)str, len, buf);
	ready = false;
	myhash.clear();
	for( int i = 0 ; i < 20 ; i++)
	{
		char buf2[3];
		if(buf[i] < 16)
			sprintf(buf2, "0%x", buf[i]);
		else
			sprintf(buf2, "%x", buf[i]);
		myhash += buf2;
	}

	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", "jabber0");
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "set");
	WokXMLTag &vtag = iqtag.AddTag("vCard");
	vtag.AddAttr("xmlns", "vcard-temp");
	vtag.GetFirstTag("FN").AddText("Kent Gustavsson");
	WokXMLTag &ptag = vtag.AddTag("PHOTO");
	ptag.AddTag("TYPE").AddText("image/jpeg");
	ptag.AddTag("BINVAL").AddText(Base64encode(str,len));
	
	delete[] str;
	wls->SendSignal("Jabber XML IQ Send", &msgtag);

	if ( !signal.empty() )
	{
		EXP_SIGUNHOOK(signal, &VCardAvatar::result, 1000);
	}
	std::string id = iqtag.GetAttr("id");
	signal = std::string("Jabber XML IQ ID ") + id;
	EXP_SIGHOOK(signal.c_str(), &VCardAvatar::result, 1000);
	
	return 1;
}

int
VCardAvatar::result(WokXMLTag *tag)
{
	ready = true;
	signal = "";
	
	return 1;
}

int
VCardAvatar::SendPresence(WokXMLTag *tag)
{
	if ( myhash.empty() && !ready)
		return 1;
	
	WokXMLTag &xtag = tag->GetFirstTag("presence").AddTag("x");
	xtag.AddAttr("xmlns", "vcard-temp:x:update");
	xtag.AddTag("photo").AddText(myhash);
	
	return 1;
}
	
int
VCardAvatar::Presence(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator tagiter; 
	
	for( tagiter = tag->GetFirstTag("presence").GetTagList("x").begin() ; tagiter != tag->GetFirstTag("presence").GetTagList("x").end() ; tagiter++ )
	{
		if ((*tagiter)->GetAttr("xmlns") == "vcard-temp:x:update")
		{
			std::string jid = tag->GetFirstTag("presence").GetAttr("from");
			jid = jid.substr(0, jid.find("/"));
			if ( user.find(jid) == user.end() )
			{
				user[jid] = new User(wls, jid);
				user[jid]->SetHash((*tagiter)->GetFirstTag("photo").GetBody());
			}
			else
				user[jid]->SetHash((*tagiter)->GetFirstTag("photo").GetBody());
				
			if ( !(*tagiter)->GetTagList("photo").empty() )
			{
				std::ifstream file;
				file.open((std::string(std::getenv("HOME"))+"/.wokjab/avatar/"+(*tagiter)->GetFirstTag("photo").GetBody()).c_str(),std::ios::in);
				if(file.is_open())
				{
					file.close();
					continue;
				}
				file.close();
				
				WokXMLTag querytag(NULL, "query");
				WokXMLTag &itemtag = querytag.AddTag("item");
				itemtag.AddAttr("jid", tag->GetFirstTag("presence").GetAttr("from"));
				itemtag.AddAttr("session", tag->GetAttr("session"));
				
				wls->SendSignal("Jabber Roster GetResource", &querytag);
					
				std::string jidresource;
				std::string jid = tag->GetFirstTag("presence").GetAttr("from");
				jid = jid.substr(0, jid.find("/"));
				if(itemtag.GetTagList("resource").size() == 0)
					jidresource =jid;
				else
				{
					if ( itemtag.GetFirstTag("resource").GetAttr("name").size() )
						jidresource = jid + '/' + itemtag.GetFirstTag("resource").GetAttr("name");
					else
						jidresource = jid;
				}
//				jidresource = jidresource.substr(0,jidresource.rfind("/"));
				
				WokXMLTag msgtag(NULL, "message");
				msgtag.AddAttr("session", tag->GetAttr("session"));
				WokXMLTag &iq_tag = msgtag.AddTag("iq");
				WokXMLTag &vcard = iq_tag.AddTag("vCard");
				iq_tag.AddAttr("to", jidresource);
				iq_tag.AddAttr("type", "get");
				vcard.AddAttr("xmlns", "vcard-temp");
				vcard.AddAttr("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
				vcard.AddAttr("version", "2.0");
				wls->SendSignal("Jabber XML IQ Send", &msgtag);
				
				EXP_SIGHOOK(std::string("Jabber XML IQ ID ") + iq_tag.GetAttr("id"), &VCardAvatar::vcard, 1000);
			}
		}
	
	}


	return 1;
}

int
VCardAvatar::GetIcon(WokXMLTag *tag)
{
	std::list <WokXMLTag*>::iterator tagiter;
	for( tagiter = tag->GetTagList("item").begin() ; tagiter != tag->GetTagList("item").end() ; tagiter++)
	{
		if ( user.find((*tagiter)->GetAttr("jid")) != user.end() )
		{
			if ( ! user[(*tagiter)->GetAttr("jid")]->GetHash().empty() )
			{
				(*tagiter)->AddAttr("avatar", (std::string(std::getenv("HOME"))+"/.wokjab/avatar/" + user[(*tagiter)->GetAttr("jid")]->GetHash()));		
			}
		}
	
	}
	
	return 1;
}

int
VCardAvatar::vcard(WokXMLTag *tag)
{
	WokXMLTag *msgtag = &tag->GetFirstTag("iq");
	WokXMLTag &vcard = msgtag->GetFirstTag("vCard");
	
	std::string jid = msgtag->GetAttr("from");
	jid = jid.substr(0, jid.find("/"));
	if ( user.find(jid) == user.end() )
	{
		woklib_error(wls, "What the fuck");
		return 1;	
	}
	
	if ( vcard.GetAttr("xmlns") != "vcard-temp")
		return 1;

	WokXMLTag &photo = vcard.GetFirstTag("PHOTO");
	std::string data = photo.GetFirstTag("BINVAL").GetBody();
	std::stringstream file;
	
	unsigned int c = 0;
	int pos = 0;
	unsigned int tmp;
	bool ended = false;
	for ( int i = 0 ; i < data.size() ; i++)
	{	
		if ( data[i] <= 'z' && data[i] >= 'a' )
		{
			tmp = data[i] - 'a' + 26;
		}
		else if ( data[i] <= 'Z' && data[i] >= 'A' )
		{
			tmp = data[i] - 'A';
		}
		else if ( data[i] >= '0' && data[i] <= '9' )
		{
			tmp = data[i] - '0' + 52;
		}
		else if ( data[i] == '+' )
		{
			tmp = 62;
		}
		else if ( data[i] == '/' )
		{
			tmp = 63;
		}
		else if ( data[i] == '=' )
		{
			ended = true;
			tmp = 0;
		}
		else
		{
			woklib_debug(wls, "avatar: base64 bad data");
			continue;
		}

		switch (pos)
		{
			case 0:
				tmp *= 64 * 64 * 64;
				break;
			case 1:
				tmp *= 64 * 64;
				break;
			case 2:
				tmp *= 64;
				break;
		}

		c += tmp;
		if ( pos == 3 || ended)
		{
			if ( ended )
			{
				pos--;
			}
			
			int spos = 2;
			for(;pos;pos--)
			{
				file << static_cast <unsigned char> ((c>>(spos--*8)) & 0xFF);
			}
			
			c = 0;
			pos = 0;
		}
		else
			pos++;
		if (ended)
			break;
	}
	
	unsigned char buffer[30];
	SHA1((unsigned char *)file.str().c_str(), file.str().size(), buffer);
	std::string hash = "";
	for( int i = 0 ; i < 20 ; i++)
	{
		char buf2[3];
		if(buffer[i] < 16)
			sprintf(buf2, "0%x", buffer[i]);
		else
			sprintf(buf2, "%x", buffer[i]);
		hash += buf2;
	}
	
	std::ofstream realfile((std::string(std::getenv("HOME"))+"/.wokjab/avatar/"+hash).c_str());
	realfile << file.str();
	realfile.close();
	
	WokXMLTag emptytag(NULL, "empty");
	wls->SendSignal("Jabber Roster Update " + tag->GetAttr("session") + " " + jid, emptytag);
	return 1;
}
