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
// Class: JabberRoster
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Sep  2 11:29:22 2006
//

#include "JabberRoster.h"

#ifndef _
#define _(x) x
#endif

JabberRoster::JabberRoster(WLSignal *wls) : WoklibPlugin(wls)
{
	config = new WokXMLTag("config");
	EXP_SIGHOOK("Config XML Change /main/window/jabber_roster", &JabberRoster::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window/jabber_roster");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	
	EXP_SIGHOOK("Jabber Event Add", &JabberRoster::AddItemEvent, 1000);
	EXP_SIGHOOK("Jabber Connection Lost", &JabberRoster::LoggedOut, 1);
	EXP_SIGHOOK("Jabber Connection Connect" , &JabberRoster::SignIn, 1000);
	
	
	#warning should get a plugin of its own 
	EXP_SIGHOOK("Wokjab XMLMarkup Parse", &JabberRoster::ParseXMLText, 1000);
	
	
	if ( config->GetFirstTag("ticker").GetAttr("data") == "false" )
	{
		WokXMLTag tag("timer");
		tag.AddAttr("time", "500");
		tag.AddAttr("signal", "Jabber Roster GUI Ticker Update");
		
		wls->SendSignal("Woklib Timmer Add",tag);
	}

	EXP_SIGHOOK("Jabber Roster GUI Ticker Update", &JabberRoster::KeepAlive, 1000);
}

JabberRoster::~JabberRoster()
{
	
}

int
JabberRoster::KeepAlive(WokXMLTag *tag)
{
	if ( config->GetFirstTag("ticker").GetAttr("data") != "false" )
	{
		tag->AddAttr("stop", "no longer needed");
		return 1;
	}
	return 1;	
}

int
JabberRoster::ReadConfig(WokXMLTag *tag)
{
	tag->GetFirstTag("config").GetFirstTag("usermarkup").AddAttr("type", "text");
	tag->GetFirstTag("config").GetFirstTag("usermarkup").AddAttr("label", _("Markup for jids in the roster"));
	if ( tag->GetFirstTag("config").GetFirstTag("usermarkup").GetFirstTag("tooltip", "config").GetBody().empty() )
		tag->GetFirstTag("config").GetFirstTag("usermarkup").GetFirstTag("tooltip", "config").AddText(_("Markup for the jids, only style tag works, the syntax can be found at http://library.gnome.org/devel/pango/unstable/PangoMarkupFormat.html\nPossibly variables are\nnick\nstatus\nshow"));
	if ( tag->GetFirstTag("config").GetFirstTag("usermarkup").GetBody().empty() )
		tag->GetFirstTag("config").GetFirstTag("usermarkup").AddText("<var name='nick'/><condition type='not empty' var='show'> - <span style='italic' size='x-small' color='blue'><var name='show'/></span></condition><condition type='not empty' var='status'>\n<span style='italic' size='x-small'><var name='status'/></span></condition>");

	tag->GetFirstTag("config").GetFirstTag("groupmarkup").AddAttr("type", "text");
	tag->GetFirstTag("config").GetFirstTag("groupmarkup").AddAttr("label", _("Markup for groups in the roster"));
	if ( tag->GetFirstTag("config").GetFirstTag("groupmarkup").GetFirstTag("tooltip", "config").GetBody().empty() )
		tag->GetFirstTag("config").GetFirstTag("groupmarkup").GetFirstTag("tooltip", "config").AddText(_("Markup for the groups, only style tag works, the syntax can be found at http://library.gnome.org/devel/pango/unstable/PangoMarkupFormat.html\nPossibly variables are\nnick\nstatus\nshow"));
	if ( tag->GetFirstTag("config").GetFirstTag("groupmarkup").GetBody().empty() )
		tag->GetFirstTag("config").GetFirstTag("groupmarkup").AddText("<var name='name'/>");
																										
	tag->GetFirstTag("config").GetFirstTag("ticker").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("ticker").AddAttr("label", "Disable Ticker");
	
	tag->GetFirstTag("config").GetFirstTag("ticker_single").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("ticker_single").AddAttr("label", "Tick single items");
	
	tag->GetFirstTag("config").GetFirstTag("display_avatar").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("display_avatar").AddAttr("label", "Display avatar in roster");
	
	if ( config->GetFirstTag("ticker").GetAttr("data") == "false" && tag->GetFirstTag("config").GetFirstTag("ticker").GetAttr("data") != "false" )
	{
		WokXMLTag tag("timer");
		tag.AddAttr("time", "500");
		tag.AddAttr("signal", "Jabber Roster GUI Ticker Update");
		
		wls->SendSignal("Woklib Timmer Add",tag);
	}
	
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));

	std::map <std::string, JabberSession *>::iterator sessiter;
	
	for ( sessiter = session.begin() ; sessiter != session.end() ; sessiter++ )
	{
		sessiter->second->UpdateAll();
	}
	return 1;
}


int
JabberRoster::ParseXMLText(WokXMLTag *tag)
{
	std::list <WokXMLObject *>::iterator oiter;
	for ( oiter = tag->GetFirstTag("markup").GetItemList().begin() ; oiter != tag->GetFirstTag("markup").GetItemList().end() ; oiter++)
	{
		switch ( (*oiter)->GetType() )
		{
			case 1:
				WokXMLTag *otag;
				otag = (WokXMLTag *)(*oiter);
				if ( otag->GetName() == "var" )
				{
					if ( !tag->GetFirstTag("variables").GetTagList(otag->GetAttr("name")).empty() )
					{
						tag->GetFirstTag("output").AddText(tag->GetFirstTag("variables").GetFirstTag(otag->GetAttr("name")).GetBody());
					}
					continue;
				}
				if ( otag->GetName() == "span" )
				{
					WokXMLTag parse("parse");
					parse.Add("<markup>" + otag->GetChildrenStr() + "</markup>");
					parse.AddTag(&tag->GetFirstTag("variables"));
					ParseXMLText(&parse);
					
					std::string spantxt = "<span";
					
					char *sargs[] = {"color", "font_desc", "font_family", "face", "font_family", "size", "style", "weight", "variant", "stretch", "foreground", "background", 
						"underline", "underline_col", "rise", "strikethrough", "strikethrough_color", "fallback", "lang", NULL };

					int x = 0;
					
					for( int x = 0 ; sargs[x] ; x++ )
					{
						if ( !otag->GetAttr(sargs[x]).empty() )
							spantxt += std::string(" ") + sargs[x] + "='" + XMLisize(otag->GetAttr(sargs[x])) + "'";
					}
					
					spantxt +=">";
					tag->GetFirstTag("output").AddText(spantxt + DeXMLisize(parse.GetFirstTag("output").GetChildrenStr()) + "</span>");
				}
				if ( otag->GetName() == "condition")
				{
					if ( otag->GetAttr("type") == "not empty" )
					{
						if ( !tag->GetFirstTag("variables").GetTagList(otag->GetAttr("var")).empty() )
						{
							if ( !tag->GetFirstTag("variables").GetFirstTag(otag->GetAttr("var")).GetBody().empty() )
							{
								WokXMLTag parse("parse");
								parse.Add("<markup>" + otag->GetChildrenStr() + "</markup>");
								parse.AddTag(&tag->GetFirstTag("variables"));
								ParseXMLText(&parse);
								
								tag->GetFirstTag("output").AddText(DeXMLisize(parse.GetFirstTag("output").GetChildrenStr()));
							}
						}
					}
					
					
				}
				break;
			case 2:
				
				WokXMLText *tt;
				tt = (WokXMLText *)(*oiter);
				tag->GetFirstTag("output").AddText(tt->GetText());
				break;
		}
	}
	return 1;
}

int
JabberRoster::Presence(WokXMLTag *tag)
{
	


	return 1;
}

int
JabberRoster::AddItemEvent(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for ( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( session.find((*iter)->GetAttr("session")) != session.end() )
			session[(*iter)->GetAttr("session")]->AddEvent(*iter);
	}

	return 1;
}

int
JabberRoster::LoggedOut(WokXMLTag *tag)
{
	if ( session.find(tag->GetAttr("session") ) != session.end() )
	{
		delete session[tag->GetAttr("session")];
		session.erase(tag->GetAttr("session"));
	}

	return 1;
}

int
JabberRoster::SignIn(WokXMLTag *tag)
{	
	session[tag->GetAttr("session")] = new JabberSession(wls, tag, this);
	
	return 1;
}
