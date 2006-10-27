/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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
// Class: ReversiHandler
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Aug 27 23:18:39 2005
//

#include "ReversiHandler.h"
#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

ReversiHandler::ReversiHandler(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML IQ New game set xmlns:http://rhymbox.com/protocol/reversi.html#1.0", &ReversiHandler::Request, 1000);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &ReversiHandler::Menu, 1000);
	EXP_SIGHOOK("Jabber Games Reversi StartSession", &ReversiHandler::StartSession, 1000);
	EXP_SIGHOOK("Jabber Games Reversi RemoveSession" , &ReversiHandler::RemoveSession, 1000);
	EXP_SIGHOOK("Jabber XML Message xmlns http://rhymbox.com/protocol/reversi.html#1.0", &ReversiHandler::Move, 1000);
}


ReversiHandler::~ReversiHandler()
{
	
}

int
ReversiHandler::Move(WokXMLTag *tag)
{
	std::string id(tag->GetFirstTag("message").GetFirstTag("thread").GetBody());
	std::string session(tag->GetAttr("session"));
	
	if( tag->GetFirstTag("message").GetFirstTag("x").GetTagList("forfeit").size() )
	{
		// std::cout << "Opponent forfeits the game" << std::endl;
		if( Session.find(session) != Session.end() && Session[session].find(id) != Session[session].end())
		{
			delete Session[session][id];
			Session[session].erase(id);
			
			if( !Session[session].size() )
				Session.erase(session);
		}
		
		return true;
	}
	
	if( id.size() )
	{
		Session[session][id]->Move(tag);
	}
	else
	{
		woklib_message(wls,"Someone is sanding you game data of a game thats currently not active");
		// Odd...
	}
	return true;
}

int
ReversiHandler::Request(WokXMLTag *tag)
{
	GladeXML *xml;
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/jabberreversi.glade", "Dialog", NULL);

	GtkWidget *dialog = glade_xml_get_widget (xml, "Dialog");
	GtkWidget *label = glade_xml_get_widget (xml, "question");
	gtk_label_set_text(GTK_LABEL(label), (tag->GetFirstTag("iq").GetAttr("from") + " \nhas requestied to play reversi with you").c_str());
	switch(gtk_dialog_run(GTK_DIALOG(dialog)))
	{
		case GTK_RESPONSE_CANCEL:
			WokXMLTag resptag(*tag);
			resptag.GetFirstTag("iq").AddAttr("to", resptag.GetFirstTag("iq").GetAttr("from"));
			resptag.GetFirstTag("iq").AddAttr("type", "error");
			resptag.GetFirstTag("iq").RemoveAttr("from");
			wls->SendSignal("Jabber XML Send", &resptag);
			gtk_widget_destroy(dialog);
	
			g_object_unref(xml);
			return true;
			break;
	}
	gtk_widget_destroy(dialog);
	
	WokXMLTag resptag(*tag);
	WokXMLTag &iqtag = resptag.GetFirstTag("iq");
	iqtag.AddAttr("to", iqtag.GetAttr("from"));
	iqtag.RemoveAttr("from");
	iqtag.AddAttr("type", "result");
	
	wls->SendSignal("Jabber XML Send", &resptag);

	Session[tag->GetAttr("session")][iqtag.GetAttr("id")] = new Game(wls, tag, false);
	g_object_unref(xml);
	return true;
}

int
ReversiHandler::IQResp(WokXMLTag *tag)
{
	if( tag->GetFirstTag("iq").GetAttr("type") == "result")
		Session[tag->GetAttr("session")][tag->GetFirstTag("iq").GetAttr("id")] = new Game(wls, tag, true);
	
	EXP_SIGUNHOOK("Jabber XML IQ ID " + tag->GetFirstTag("iq").GetAttr("id"), &ReversiHandler::IQResp, 1000);
		
	return true;
}

int
ReversiHandler::StartSession(WokXMLTag *tag)
{
	std::string mto = tag->GetAttr("jid");
	
	if( mto.find("/") == std::string::npos )
	{
		WokXMLTag restag(NULL, "query");
		WokXMLTag &itemtag = restag.AddTag("item");
		itemtag.AddAttr("jid", mto);
		itemtag.AddAttr("session", tag->GetAttr("session"));
		
		wls->SendSignal("Jabber Roster GetResource", &restag);
		mto = mto + "/" + itemtag.GetFirstTag("resource").GetAttr("name");		
	}
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddTag("game").AddAttr("xmlns", "http://rhymbox.com/protocol/reversi.html#1.0");
	iqtag.AddAttr("to", mto);
	iqtag.AddAttr("type", "set");
	
	wls->SendSignal("Jabber XML IQ Send", msgtag);
	EXP_SIGHOOK("Jabber XML IQ ID " + iqtag.GetAttr("id"), &ReversiHandler::IQResp, 1000);
	
	return true;
}

int
ReversiHandler::RemoveSession(WokXMLTag *tag)
{
	std::string id(tag->GetAttr("id"));
	std::string session(tag->GetAttr("session"));
	if( Session.find(session) != Session.end() && Session[session].find(id) != Session[session].end())
	{
		delete Session[session][id];
		Session[session].erase(id);
		
		if( !Session[session].size() )
			Session.erase(session);
	}

	return true;
}

int
ReversiHandler::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Reversi");
	item.AddAttr("signal", "Jabber Games Reversi StartSession");
	return true;
}
