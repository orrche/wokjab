/***************************************************************************
 *  Copyright (C) 2006-2008  Kent Gustavsson <nedo80@gmail.com>
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

#include <cstdlib>
#include <gtk/gtk.h>

#include "openssl/sha.h"
const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


VCardAvatar::VCardAvatar(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Avatar SetMy", &VCardAvatar::GetMyCard, 50);
	EXP_SIGHOOK("Jabber XML Presence", &VCardAvatar::Presence, 50);
	EXP_SIGHOOK("Jabber XML Presence Send", &VCardAvatar::SendPresence, 50);
	EXP_SIGHOOK("Jabber GUI GetIcon", &VCardAvatar::GetIcon, 50);
	EXP_SIGHOOK("Jabber Connection Authenticated", &VCardAvatar::NewSession, 50);
	
	EXP_SIGHOOK("GetMenu", &VCardAvatar::MainMenu, 999);
	EXP_SIGHOOK("Jabber Avatar MenuSet", &VCardAvatar::MenuSet, 50);
	
	mkdir((std::string(std::getenv("HOME")) + "/.wokjab").c_str(), 0700);
	mkdir((std::string(std::getenv("HOME")) + "/.wokjab/avatar").c_str(), 0700);
	
	mypictag = NULL;
	gxml = NULL;
	ready = false;
}


VCardAvatar::~VCardAvatar()
{

}

int
VCardAvatar::MenuSet(WokXMLTag *tag)
{
	if ( !gxml )
	{
		gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/vcardavatar.glade", NULL, NULL);
		g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "ok_button")), "clicked",
			G_CALLBACK (VCardAvatar::OK_Button), this);
		g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "close_button")), "clicked",
			G_CALLBACK (VCardAvatar::Close_Button), this);
		g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "window")), "destroy", 
			G_CALLBACK (VCardAvatar::Window_Destroy), this);
	
	}

	gtk_window_present(GTK_WINDOW(glade_xml_get_widget(gxml, "window")));
}

void
VCardAvatar::Window_Destroy (GtkObject *object, VCardAvatar *c)
{
	g_object_unref(c->gxml);
	c->gxml = NULL;
}

void
VCardAvatar::OK_Button(GtkButton *button, VCardAvatar *c)
{
	gchar *file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(glade_xml_get_widget (c->gxml, "file")));
	
	if ( file_name )
	{
		WokXMLTag avatar("avatar");
		avatar.AddAttr("file", file_name);
		
		if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (c->gxml, "checkbuttonReformat"))) == TRUE)
			avatar.AddAttr("reformat", "true");
		else
			avatar.AddAttr("reformat", "false");
		
		c->wls->SendSignal("Jabber Avatar SetMy", avatar);
	
		g_free(file_name);
	}
	
	gtk_widget_destroy(glade_xml_get_widget (c->gxml, "window"));

}

void
VCardAvatar::Close_Button(GtkButton *button, VCardAvatar *c)
{
	gtk_widget_destroy(glade_xml_get_widget (c->gxml, "window"));
}

int
VCardAvatar::MainMenu(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	WokXMLTag *settingstag = NULL;
	for ( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("name") == "Settings")
		{
			settingstag = *iter;
			break;
		}
	}
	if( !settingstag )
	{
		settingstag = &tag->AddTag("item");
		settingstag->AddAttr("name", "Settings");
	}
	
	WokXMLTag *xml;
	xml = &settingstag->AddTag("item");
	xml->AddAttr("name", "Set Avatar");
	xml->AddAttr("signal", "Jabber Avatar MenuSet");
	
	return 1;
}

std::string
VCardAvatar::Base64encode(const unsigned char *buf, int len)
{
	std::string ret("");
	int n = 0;
	
	for ( int i = (int)((2.99 + len)/3) ; i ; i-- )
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
VCardAvatar::MyVcard(WokXMLTag *tag)
{
	if ( tag->GetFirstTag("iq").GetAttr("type") != "result" )
		return 1;
	
	WokXMLTag &binval = tag->GetFirstTag("iq").GetFirstTag("vCard", "vcard-temp").GetFirstTag("PHOTO").GetFirstTag("BINVAL");
	
	if ( binval.GetBody().empty() )
		return 1;
		
	unsigned char buffer[binval.GetBody().size()];
	int len = binval.GetBodyAsBase64((char*)buffer, binval.GetBody().size());
	
	unsigned char buf[30];
	SHA1(buffer,len,buf);
	
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
	
	WokXMLTag mess("message");
	mess.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &ptag = mess.AddTag("presence");

	wls->SendSignal("Jabber XML Presence Send", mess);
	
	return 1;
}

int
VCardAvatar::NewSession(WokXMLTag *tag)
{
	std::string session = tag->GetAttr("session");
	
	WokXMLTag data("data");
	data.AddTag("item").AddAttr("session", session);
	wls->SendSignal("Jabber Connection GetUserData", data);
	
	WokXMLTag msgtag("message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "get");
	iqtag.AddAttr("to", data.GetFirstTag("item").GetFirstTag("username").GetBody() + "@" + data.GetFirstTag("item").GetFirstTag("server").GetBody());
	
	WokXMLTag &vcard = iqtag.AddTag("vCard", "vcard-temp");
	vcard.AddAttr("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
	vcard.AddAttr("version", "2.0");
	
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	EXP_SIGHOOK("Jabber XML IQ ID " + msgtag.GetFirstTag("iq").GetAttr("id"), &VCardAvatar::MyVcard, 1000);

	return 1;
}

int
VCardAvatar::GetMyCard(WokXMLTag *tag)
{
	delete mypictag;
	mypictag = new WokXMLTag(*tag);
		
	WokXMLTag sess("session");
	wls->SendSignal("Jabber GetSessions", &sess);
	        
	std::list <WokXMLTag *>::iterator iter;
	
	for ( iter = sess.GetTagList("item").begin() ; iter != sess.GetTagList("item").end(); iter++)
	{
		std::string session = (*iter)->GetAttr("name");
	
		WokXMLTag data("data");
		data.AddTag("item").AddAttr("session", session);
		wls->SendSignal("Jabber Connection GetUserData", data);
	
		WokXMLTag msgtag("message");
		msgtag.AddAttr("session", session);
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("type", "get");
		iqtag.AddAttr("to", data.GetFirstTag("item").GetFirstTag("username").GetBody() + "@" + data.GetFirstTag("item").GetFirstTag("server").GetBody());
	
		WokXMLTag &vcard = iqtag.AddTag("vCard", "vcard-temp");
		vcard.AddAttr("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
		vcard.AddAttr("version", "2.0");
	
	
		wls->SendSignal("Jabber XML IQ Send", &msgtag);
		EXP_SIGHOOK("Jabber XML IQ ID " + msgtag.GetFirstTag("iq").GetAttr("id"), &VCardAvatar::SetMy, 1000);
	}

	
	return 1;	
}

int
VCardAvatar::SetMy(WokXMLTag *tag)
{
	//GError* err;		
	
	gchar *str;
	gsize len; 
	
	if ( mypictag->GetAttr("reformat") != "false" )
	{
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (mypictag->GetAttr("file").c_str(), NULL);
		
		if ( pixbuf == NULL )
			return 1;
		
		int pwidth = gdk_pixbuf_get_width(pixbuf);
		int pheight = gdk_pixbuf_get_height(pixbuf);
		int w,h;
		
		if ( pwidth > pheight )
		{
			w = 128;
			h = int(pheight / ( ((double)pwidth) / 128 ));
		}
		else
		{
			w = int(pwidth / ( ((double)pheight) / 128 ));
			h = 128;
		}
		
		GdkPixbuf *scaled = gdk_pixbuf_scale_simple (pixbuf, w, h, GDK_INTERP_BILINEAR);
		g_object_unref(pixbuf);
		
		if ( scaled == NULL )
		{
			return 1;
		}
		
		gboolean ret = gdk_pixbuf_save_to_buffer (scaled, &str, &len, "png", NULL, NULL);
		g_object_unref(scaled);
	}
	else
	{
		FILE *f = fopen(mypictag->GetAttr("file").c_str(), "rb");
		str = (gchar*) g_malloc(sizeof(char)*8000);
		len = fread(str, 1, 8000, f);
		fclose(f);		
	}
	
	unsigned char buf[30];
	SHA1((unsigned char*)str, len, buf);
	
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
	WokXMLTag &vtag = iqtag.AddTag(&tag->GetFirstTag("iq").GetFirstTag("vCard", "vcard-temp"));
	
	WokXMLTag &ptag = vtag.GetFirstTag("PHOTO");
	ptag.GetFirstTag("TYPE").RemoveBody();
	ptag.GetFirstTag("TYPE").AddText("image/png");
	ptag.GetFirstTag("BINVAL").RemoveBody();
	ptag.GetFirstTag("BINVAL").AddText(Base64encode(( const unsigned char *)str,len));
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);

	g_free(str);
	
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
				
				std::string jid = tag->GetFirstTag("presence").GetAttr("from");
				std::string jidresource;
				if ( jid.find("/") != std::string::npos ) 
				{
					jidresource = jid;
					jid = jid.substr(0, jid.find("/"));
				}
				else
				{
					WokXMLTag querytag(NULL, "query");
					WokXMLTag &itemtag = querytag.AddTag("item");
					itemtag.AddAttr("jid", jid);
					itemtag.AddAttr("session", tag->GetAttr("session"));
				
					wls->SendSignal("Jabber Roster GetResource", &querytag);
					
					if(itemtag.GetTagList("resource").size() == 0)
						jidresource =jid;
					else
					{
						if ( itemtag.GetFirstTag("resource").GetAttr("name").size() )
							jidresource = jid + '/' + itemtag.GetFirstTag("resource").GetAttr("name");
						else
							jidresource = jid;
					}
				}
				
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
		woklib_error(wls, "-------");
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
