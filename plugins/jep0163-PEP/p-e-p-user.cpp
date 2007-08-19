/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "p-e-p-user.hpp"

PEPUser::PEPUser(WLSignal *wls, PEP_Widget *pw, std::string session, std::string jid) : WLSignalInstance(wls), 
pw(pw), session(session), jid(jid)
{
	visible = false;
	{
		WokXMLTag mtag ("message");
		mtag.AddAttr("session", session);
		WokXMLTag &iqtag = mtag.AddTag("iq");
		iqtag.AddAttr("to", jid);
		iqtag.AddAttr("type", "get");
		iqtag.AddTag("query").AddAttr("xmlns", "http://jabber.org/protocol/disco#items");


		wls->SendSignal("Jabber XML IQ Send", mtag);
		EXP_SIGHOOK("Jabber XML IQ ID " + mtag.GetFirstTag("iq").GetAttr("id"), &PEPUser::UserQueryResultItem, 1000);
	}
	{
		WokXMLTag mtag("message");
		mtag.AddAttr("session", session);
		WokXMLTag &iqtag = mtag.AddTag("iq");
		iqtag.AddAttr("type", "get");
		iqtag.AddAttr("to", jid);
		WokXMLTag &pubsub = iqtag.AddTag("pubsub");
		pubsub.AddAttr("xmlns", "http://jabber.org/protocol/pubsub");
		pubsub.AddTag("subscriptions");
		
		wls->SendSignal("Jabber XML IQ Send", mtag);
		EXP_SIGHOOK("Jabber XML IQ ID " + mtag.GetFirstTag("iq").GetAttr("id"), &PEPUser::UserQuerySubscription, 1000);
	}					
}

PEPUser::~PEPUser()
{
	
	
	
}

bool
PEPUser::SetNode(std::string node)
{
	if ( nodelist.find(node) == nodelist.end() )
		return false;
	
	currentnode = node;
	return true;	
}

void
PEPUser::Show()
{
	if ( !visible )
		gtk_list_store_append(pw->jid_list, &tIter);
	visible = true;
	Update();	
	
}

void
PEPUser::Hide()
{
	if ( visible )
		gtk_list_store_remove(pw->jid_list, &tIter);
	visible = false;	
}

void
PEPUser::Update()
{
	if ( nodelist[currentnode] == 1 )
		gtk_list_store_set (pw->jid_list, &tIter,
								0, TRUE,
								1, jid.c_str(),
								-1);
	else
		gtk_list_store_set (pw->jid_list, &tIter,
								0, FALSE,
								1, jid.c_str(),
								-1);
		
}

int
PEPUser::UserQuerySubscription(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator ps_iter;
	std::cout << " HERE #####################" << std::endl;
	for ( ps_iter = tag->GetFirstTag("iq").GetTagList("pubsub").begin() ; ps_iter != tag->GetFirstTag("iq").GetTagList("pubsub").end() ; ps_iter++)
	{
		if ( (*ps_iter)->GetAttr("xmlns") == "http://jabber.org/protocol/pubsub" )
		{
			std::cout << "Pubsub.." << std::endl;
			std::list <WokXMLTag *>::iterator sub_iter;
			for ( sub_iter = (*ps_iter)->GetFirstTag("subscriptions").GetTagList("subscription").begin() ;
					sub_iter != (*ps_iter)->GetFirstTag("subscriptions").GetTagList("subscription").end() ; sub_iter++)
			{
				if ( (*sub_iter)->GetAttr("subscription") == "subscribed" )
					nodelist[(*sub_iter)->GetAttr("node")] = 1;
				else
					nodelist[(*sub_iter)->GetAttr("node")] = 0;
				
				pw->AddType((*sub_iter)->GetAttr("node"));
				
				if ( visible ) 
					Update();
			}
		}
		
	}
	
	/*
	<iq type='result'
		from='pubsub.shakespeare.lit'
		to='francisco@denmark.lit'
		id='subscriptions1'>
	  <pubsub xmlns='http://jabber.org/protocol/pubsub'>
		<subscriptions>
		  <subscription node='node1' jid='francisco@denmark.lit' subscription='subscribed'/>
		  <subscription node='node2' jid='francisco@denmark.lit' subscription='subscribed'/>
		  <subscription node='node5' jid='francisco@denmark.lit' subscription='unconfigured'/>
		  <subscription node='node6' jid='francisco@denmark.lit' subscription='pending'/>
		</subscriptions>
	  </pubsub>
	</iq>
	*/
	
	return 1;	
}

int
PEPUser::UserQueryResultInfo(WokXMLTag *tag)
{	
	std::list <WokXMLTag *>::iterator queryiter;
	for ( queryiter = tag->GetFirstTag("iq").GetTagList("query").begin() ; queryiter != tag->GetFirstTag("iq").GetTagList("query").end() ; queryiter++) 
	{
		if ( (*queryiter)->GetAttr("xmlns") == "http://jabber.org/protocol/disco#info" )
		{
			std::list <WokXMLTag *>::iterator itemiter;
			for( itemiter = (*queryiter)->GetTagList("identity").begin() ; itemiter != (*queryiter)->GetTagList("identity").end() ; itemiter++)
			{
				if ( (*itemiter)->GetAttr("category") == "pubsub" )
				{
					if ( nodelist.find((*queryiter)->GetAttr("node")) == nodelist.end() )
					{
						nodelist[(*queryiter)->GetAttr("node")] = -1;
						if ( visible ) 
							Update();
						pw->AddType((*queryiter)->GetAttr("node"));
						
						/*
						<iq type='get'
							from='francisco@denmark.lit/barracks'
							to='pubsub.shakespeare.lit'
							id='subscriptions1'>
						  <pubsub xmlns='http://jabber.org/protocol/pubsub'>
							<subscriptions/>
						  </pubsub>
						</iq>*/
					}
						
				}
			}
		}
	}
	return 1;
}

int
PEPUser::UserQueryResultItem(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator queryiter;
	for ( queryiter = tag->GetFirstTag("iq").GetTagList("query").begin() ; queryiter != tag->GetFirstTag("iq").GetTagList("query").end() ; queryiter++) 
	{
		if ( (*queryiter)->GetAttr("xmlns") == "http://jabber.org/protocol/disco#items" )
		{
			std::list <WokXMLTag *>::iterator itemiter;
			for( itemiter = (*queryiter)->GetTagList("item").begin() ; itemiter != (*queryiter)->GetTagList("item").end() ; itemiter++)
			{
				WokXMLTag mtag ("message");
				mtag.AddAttr("session", session);
				WokXMLTag &iqtag = mtag.AddTag("iq");
				iqtag.AddAttr("to", jid);
				iqtag.AddAttr("type", "get");
				WokXMLTag &querytag = iqtag.AddTag("query");
				querytag.AddAttr("xmlns", "http://jabber.org/protocol/disco#info");
				querytag.AddAttr("node", "http://jabber.org/protocol/tune");
				
				wls->SendSignal("Jabber XML IQ Send", mtag);
				EXP_SIGHOOK("Jabber XML IQ ID " + mtag.GetFirstTag("iq").GetAttr("id"), &PEPUser::UserQueryResultInfo, 1000);
			}
		}
		
	}
	/*
	<iq from='juliet@capulet.com'
		to='benvolio@montague.net/home'
		id='disco3'
		type='result'>
	  <query xmlns='http://jabber.org/protocol/disco#items'>
		<item jid='juliet@capulet.com' node='http://jabber.org/protocol/tune'/>
	  </query>
	</iq>
	*/
  	return 1;
}
