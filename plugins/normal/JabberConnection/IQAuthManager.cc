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
// Class: IQAuthManager
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Nov 21 20:53:53 2006
//

#include "IQAuthManager.h"
#include "IQauth.h"

IQAuthManager::IQAuthManager(WLSignal *wls) : WLSignalInstance(wls)
{
	EXP_SIGHOOK("Jabber XML Object stream:features", &IQAuthManager::Feature, 550);
}


IQAuthManager::~IQAuthManager()
{
	
}

int
IQAuthManager::Feature(WokXMLTag *tag)
{
	if ( tag->GetFirstTag("stream:features").GetTagList("bind").size() )
	{
		WokXMLTag query(NULL, "query");
		WokXMLTag &item = query.AddTag("item");
		item.AddAttr("session", tag->GetAttr("session"));
		
		wls->SendSignal("Jabber Connection GetUserData", query);
		
		WokXMLTag msgtag(NULL,"message");
		msgtag.AddAttr("session", tag->GetAttr("session"));
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("type", "set");
		WokXMLTag &bindtag = iqtag.AddTag("bind");
		bindtag.AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");
		bindtag.AddTag("resource").AddText(item.GetFirstTag("resource").GetBody());
		
		wls->SendSignal("Jabber XML IQ Send", &msgtag);
		
		EXP_SIGHOOK("Jabber XML IQ ID " + iqtag.GetAttr("id"), &IQAuthManager::BindResp, 500);
		
	}
	else if ( !tag->GetFirstTag("stream:features").GetTagList("mechanisms").empty() )
	{
		std::list <WokXMLTag *> &list = tag->GetFirstTag("stream:features").GetFirstTag("mechanisms").GetTagList("mechanism");
		std::list <WokXMLTag *>::iterator iter;
		bool plain_possibule = false;
		
		for ( iter = list.begin() ; iter != list.end() ; iter++ )
		{			
			if ( (*iter)->GetBody() == "PLAIN" )
				plain_possibule = true;
			else if ( (*iter)->GetBody() == "DIGEST-MD5" )
			{
					/* This seams to be broken... */
				new IQauth(wls, tag->GetAttr("session"), IQauth::SASLDIGESTMD5);
				break;
			}
		}
		if ( iter == list.end() )
		{
			if ( plain_possibule )
			{
				new IQauth(wls, tag->GetAttr("session"), IQauth::SASLPLAIN);
			}
			else
			{
				woklib_error(wls, "No known loginmethods found");
				// not doing a long shot
				// new IQauth(wls, tag->GetAttr("session"), IQauth::ClearTextUser);
			}
		}
		
		
	}

}

int
IQAuthManager::BindResp(WokXMLTag *tag)
{
	WokXMLTag jidsettag(NULL, "set");
	jidsettag.AddTag("jid").AddText(tag->GetFirstTag("iq").GetFirstTag("bind").GetFirstTag("jid").GetBody());
	wls->SendSignal("Jabber Connection SetJID " + tag->GetAttr("session"), jidsettag);
	
	WokXMLTag iqmsgtag(NULL, "message");
	iqmsgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &itag = iqmsgtag.AddTag("iq");
	itag.AddAttr("type","set");
	itag.AddTag("session").AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-session");
	wls->SendSignal("Jabber XML Send", iqmsgtag);
	
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	wls->SendSignal("Jabber Connection Authenticated", &msgtag);

	return 1;
}

