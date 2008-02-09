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

#include "roster-event.hpp"
#include <sstream>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

RosterEvent::RosterEvent(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML Presence", &RosterEvent::Presence, 90); // Important need to be before the signal in the roster
	CreateWid();

	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /main/window/roster_event", &RosterEvent::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window/roster_event");
	wls->SendSignal("Config XML Trigger", &conftag);


	EXP_SIGHOOK("GUI Window Init", &RosterEvent::GUIWindowInit, atoi(config->GetFirstTag("pos").GetAttr("data").c_str()));
}

RosterEvent::~RosterEvent()
{
	EXP_SIGUNHOOK("Config XML Change /main/window/roster_event", &RosterEvent::ReadConfig, 500);
	SaveConfig();

}

void
RosterEvent::SaveConfig()
{
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window/roster_event");
	conftag.AddTag(config);
	
	wls->SendSignal("Config XML Store", &conftag);
}

int
RosterEvent::ReadConfig(WokXMLTag *tag)
{
	delete config;

	tag->GetFirstTag("config").GetFirstTag("history_size").AddAttr("type", "string");
	tag->GetFirstTag("config").GetFirstTag("history_size").AddAttr("label", "History Size");
	tag->GetFirstTag("config").GetFirstTag("timestamp_format").AddAttr("type", "string");
	tag->GetFirstTag("config").GetFirstTag("timestamp_format").AddAttr("label", "Timestamp Format");
	tag->GetFirstTag("config").GetFirstTag("hasgrip").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("hasgrip").AddAttr("label", "Has resizegrip");
	tag->GetFirstTag("config").GetFirstTag("pos").AddAttr("type", "string");
	tag->GetFirstTag("config").GetFirstTag("pos").AddAttr("label", "Position in the main window");

	if ( tag->GetFirstTag("config").GetFirstTag("pos").GetAttr("data").empty() )
		tag->GetFirstTag("config").GetFirstTag("pos").AddAttr("data", "900");
	if ( tag->GetFirstTag("config").GetFirstTag("history_size").GetAttr("data").empty() )
		tag->GetFirstTag("config").GetFirstTag("history_size").AddAttr("data", "40");
	if ( tag->GetFirstTag("config").GetFirstTag("timestamp_format").GetBody().empty() )
		tag->GetFirstTag("config").GetFirstTag("timestamp_format").AddText("[%H:%M]");
	
	config = new WokXMLTag (tag->GetFirstTag("config"));
	
	if ( tag->GetFirstTag("config").GetFirstTag("hasgrip").GetAttr("data") != "false" )
		gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(glade_xml_get_widget (xml, "statusbar")), TRUE);
	else
		gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(glade_xml_get_widget (xml, "statusbar")), FALSE);
	return 1;
}

void
RosterEvent::CreateWid()
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/roster-event.glade", "mainbox", NULL);
	GtkWidget *main_vbox = glade_xml_get_widget (xml, "mainbox");
	char buf[40];

	mainwindowplug = gtk_plug_new(0);
	
	gtk_container_add(GTK_CONTAINER(mainwindowplug), main_vbox);
	
	gtk_widget_show_all(mainwindowplug);

	WokXMLTag contag(NULL, "connect");
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(mainwindowplug)));
	WokXMLTag &widtag = contag.AddTag("widget");
	widtag.AddAttr("id", buf);
	widtag.AddAttr("expand", "false");
	widtag.AddAttr("fill", "true");
	
	if ( !wls->SendSignal("GUI Window AddWidget",&contag) )
	{
		inittag = new WokXMLTag (contag);
	}
	else
	{
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
		EXP_SIGHOOK(sig.str(), &RosterEvent::Close, 500);
	}
	contextid = gtk_statusbar_get_context_id(GTK_STATUSBAR(glade_xml_get_widget (xml, "statusbar")), "Last event");
}

std::string
RosterEvent::GetTimeStamp(time_t t)
{
	struct tm *tm;
	char      buf[128];

	tm = localtime (&t);

	buf[0] = 0;

	strftime (buf, sizeof (buf), config->GetFirstTag("timestamp_format").GetAttr("data").c_str(), tm);
	return buf;
}

int
RosterEvent::GUIWindowInit(WokXMLTag *tag)
{
	if( inittag )
	{
		wls->SendSignal("GUI Window AddWidget",inittag);
		delete inittag;
		inittag = NULL;
		
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
		EXP_SIGHOOK(sig.str(), &RosterEvent::Close, 500);
	}
	return 1;
}

int 
RosterEvent::Presence(WokXMLTag *tag)
{
	std::string from = tag->GetFirstTag("presence").GetAttr("from");
	std::string session = tag->GetAttr("session");

	WokXMLTag old_pre(NULL, "presence");
	WokXMLTag &itemtag = old_pre.AddTag("item");
	itemtag.AddAttr("session", session);
	itemtag.AddAttr("jid", from);
	wls->SendSignal("Jabber Roster GetResource", old_pre);

	std::string message;
	std::string nick = itemtag.GetAttr("nick");
	
	if (  tag->GetFirstTag("presence").GetAttr("type") == "error" || itemtag.GetAttr("inroster") != "true" )
	{
		return 1;
	}

	if ( itemtag.GetAttr("presence") == "unavailable" && tag->GetFirstTag("presence").GetAttr("type") != "unavailable" )
	{
		std::stringstream str;
		if( nick.empty() )
			str << from << " has signed in";
		else
			str << nick << " has signed in";

		message = str.str();
	}
	else if ( itemtag.GetAttr("presence") != "unavailable" && tag->GetFirstTag("presence").GetAttr("type") == "unavailable" )
	{
		std::stringstream str;
		if( nick.empty() )
			str << from << " has signed out";
		else
			str << nick << " has signed out";

		message = str.str();
	}
	else
		return 1;

	message = GetTimeStamp(time(0)) + message;

	gtk_statusbar_pop(GTK_STATUSBAR(glade_xml_get_widget (xml, "statusbar")), contextid);
	gtk_statusbar_push (GTK_STATUSBAR(glade_xml_get_widget (xml, "statusbar")), contextid, message.c_str());
	last_events.push_back(message);

	std::stringstream tt;
	std::list <std::string>::iterator iter;
	for( iter = last_events.begin() ; iter != last_events.end() ; iter++)
	{
		if ( iter != last_events.begin() )
			tt << std::endl;
		tt << *iter;
	}

	while ( last_events.size() > static_cast<unsigned int>(atoi(config->GetFirstTag("history_size").GetAttr("data").c_str()) ))
		last_events.pop_front();

	GtkTooltips *tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips,mainwindowplug, tt.str().c_str(), tt.str().c_str());
	return 1;
}

int
RosterEvent::Close(WokXMLTag *tag)
{
	gtk_widget_destroy(mainwindowplug);
	g_object_unref(xml);
	xml = NULL;

	return 1;
}
