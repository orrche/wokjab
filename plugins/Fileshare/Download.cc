/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "Download.h"

DownloadFolder::DownloadFolder(WLSignal *wls, WokXMLTag *tag, std::string jid, std::string session, std::string d_path) : WLSignalInstance(wls),
d_path(d_path),
folder(folder),
jid(jid),
session(session)
{
	if ( tag->GetAttr("name").find("/") != std::string::npos )
		GetFiles(d_path + "/" + tag->GetAttr("name").substr(tag->GetAttr("name").rfind("/") + 1 ), tag);
	else
		GetFiles(d_path + "/" + tag->GetAttr("name"), tag);

	std::map <std::string, std::string>::iterator iter;
	
	if( !list.empty() )
	{
		DownloadFile( *list.begin());
	}
	
	
	EXP_SIGHOOK("Jabber Stream File Status Finished", &DownloadFolder::Finished, 1000);
	EXP_SIGHOOK("Jabber Stream File Status Terminated", &DownloadFolder::Finished, 1000);
	EXP_SIGHOOK("Jabber Stream File Status Rejected", &DownloadFolder::Finished, 1000);
}

DownloadFolder::~DownloadFolder()
{
	


}

void
DownloadFolder::GetFiles(std::string path, WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> *flist;
	
	flist = &tag->GetTagList("item");
	for( iter = flist->begin() ; iter != flist->end() ; iter++)
	{
		if ( (*iter)->GetAttr("type") == "folder")
		{
				GetFiles(path + "/" + (*iter)->GetAttr("name"), (*iter));
		}
		else if ( (*iter)->GetAttr("type") == "file")
		{
			std::vector <std::string> vect;
			vect.push_back((*iter)->GetAttr("id"));
			vect.push_back(path + '/' + (*iter)->GetAttr("name"));
			
			list.push_back(vect);
		}
	}
}

int
DownloadFolder::Finished(WokXMLTag *tag)
{		
	if ( tag->GetAttr("sid") == sid)
	{
		list.pop_front();
		if( list.begin() == list.end() )
			delete this;
		else
			DownloadFile( *list.begin() );
	}
	return 1;
}

void
DownloadFolder::DownloadFile(std::vector<std::string> vect)
{

			WokXMLTag msg(NULL, "message");
			msg.AddAttr("session", session);
			WokXMLTag &iq = msg.AddTag("iq");
			iq.AddAttr("to", jid);
			iq.AddAttr("type", "get");
			WokXMLTag &fileshare = iq.AddTag("fileshare");
			fileshare.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
			WokXMLTag &file = fileshare.AddTag("file");
			file.AddAttr("id", vect[0]);
			
			wls->SendSignal("Jabber XML IQ Send", msg);
			
			if ( vect[1].find("/") != std::string::npos )
			{
				File *f;
				f = new File(wls, msg, vect[1].substr(vect[1].rfind("/") + 1), vect[1].substr(0,vect[1].rfind("/")));
				sid = f->lsid;
			}
			else
			{
				File *f;
				f = new File(wls, msg, vect[1].substr(vect[1].rfind("/") + 1),"");
				sid = f->lsid;
			}
}

