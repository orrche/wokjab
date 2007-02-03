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
// Class: MUCBookmark
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Feb  1 12:33:14 2007
//

#include "MUCBookmark.h"


MUCBookmark::MUCBookmark(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Connection Authenticated" , &MUCBookmark::SignIn, 2000);
}


MUCBookmark::~MUCBookmark()
{

}

int
MUCBookmark::SignIn(WokXMLTag *tag)
{
	WokXMLTag msg_tag(NULL, "message");
	msg_tag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iq_tag = msg_tag.AddTag("iq");
	iq_tag.AddAttr("type", "get");
	WokXMLTag &query_tag = iq_tag.AddTag("query");
	query_tag.AddAttr("xmlns", "jabber:iq:private");
	WokXMLTag &storage_tag = query_tag.AddTag("storage");
	storage_tag.AddAttr("xmlns", "storage:bookmarks");
	
	
	wls->SendSignal("Jabber XML IQ Send", msg_tag);
	EXP_SIGHOOK("Jabber XML IQ ID " + iq_tag.GetAttr("id"), &MUCBookmark::BookmarkResponse, 1000);

	return 1;
}

int
MUCBookmark::BookmarkResponse(WokXMLTag *tag)
{
	std::list <WokXMLTag *> *tags;
	std::list <WokXMLTag *>::iterator iter;
	tags = &tag->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("storage").GetTagList("conference");
	for ( iter = tags->begin() ; iter != tags->end() ; iter++)
	{
		std::string jid = (*iter)->GetAttr("jid");
		if ( jid.find("@") != std::string::npos && jid.find("/") == std::string::npos)
		{
			std::string nick = (*iter)->GetFirstTag("nick").GetBody();
			if ( nick == "" )
			{
				nick ="Unknown..";
			}
			std::string room = (*iter)->GetAttr("jid").substr(0,(*iter)->GetAttr("jid").find("@"));
			std::string server = (*iter)->GetAttr("jid").substr((*iter)->GetAttr("jid").find("@")+1);
			std::string password = (*iter)->GetFirstTag("password").GetBody();
			
			if ( (*iter)->GetAttr("autojoin") == "true" ||  (*iter)->GetAttr("autojoin") == "1" )
			{
				WokXMLTag jointag(NULL, "groupchat");
				jointag.AddAttr("nick", nick);
				jointag.AddAttr("room", room);
				jointag.AddAttr("server", server);
				jointag.AddAttr("session", tag->GetAttr("session"));
				if ( !password.empty() )
				{
					jointag.AddAttr("password", password);
				}
				if ( (*iter)->GetAttr("minimized") == "true" ||  (*iter)->GetAttr("minimized") == "1" )
					jointag.AddAttr("minimized", "true");
					
				wls->SendSignal("Jabber GroupChat Join", &jointag);
				
			}
		}
	}
	/*
	<iq type='result' id='2'>
  <query xmlns='jabber:iq:private'>
    <storage xmlns='storage:bookmarks'>
      <conference name='Council of Oberon' 
                  autojoin='true'
                  jid='council@conference.underhill.org'>
        <nick>Puck</nick>
        <password>titania</password>
      </conference>
    </storage>
  </query>
</iq>   
	
*/
}
