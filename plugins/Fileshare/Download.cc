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
	GetFiles(d_path, tag);

	std::map <std::string, std::string>::iterator iter;
	
	for( iter = list.begin() ; iter != list.end() ; iter++)
	{
		std::cout << iter->first << " = " << iter->second << std::endl;
	}
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
				list[(*iter)->GetAttr("id")] = path + "/" + (*iter)->GetAttr("name");
		}
	}
}

void
DownloadFolder::DownloadFile(std::string id, std::string path)
{
			WokXMLTag msg(NULL, "message");
			msg.AddAttr("session", session);
			WokXMLTag &iq = msg.AddTag("iq");
			iq.AddAttr("to", jid);
			iq.AddAttr("type", "get");
			WokXMLTag &fileshare = iq.AddTag("fileshare");
			fileshare.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
			WokXMLTag &file = fileshare.AddTag("file");
			file.AddAttr("id", id);
			
			wls->SendSignal("Jabber XML IQ Send", msg);
			
			file = File(wls, msg, list[id], path);
}

