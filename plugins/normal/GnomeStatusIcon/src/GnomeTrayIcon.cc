/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/**
 * \file GnomeTrayIcon.cc
 * \author Kent Gustavsson
 * \date 2006-03-24
 */
 
#include <iostream>
#include <cassert>
#include <sstream>

#include "../include/GnomeTrayIcon.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GnomeTrayIcon::GnomeTrayIcon(WLSignal *wls) : WoklibPlugin (wls)
{
	EXP_SIGHOOK("Jabber Event Add", &GnomeTrayIcon::AddJIDEvent, 1000);
	EXP_SIGHOOK("Jabber Event Remove", &GnomeTrayIcon::RemoveJIDEvent, 2);
	EXP_SIGHOOK("Jabber XML Presence Send", &GnomeTrayIcon::Presence, 1000);
	
	/*
	eventbox 	= gtk_event_box_new();
	tray_icon_tips = gtk_tooltips_new();

	gtk_container_add (GTK_CONTAINER (tray_icon), eventbox);
*/
	
	
	tray_icon 	= gtk_status_icon_new_from_file(PACKAGE_DATA_DIR"/wokjab/online.png");
	gtk_status_icon_set_has_tooltip(tray_icon, TRUE);
		/*
	gtk_tooltips_disable(GTK_TOOLTIPS(tray_icon_tips));
	*/ 
	

	g_signal_connect (G_OBJECT (tray_icon), "activate",
				G_CALLBACK (GnomeTrayIcon::tray_icon_pressed), this);
	g_signal_connect (G_OBJECT (tray_icon), "popup-menu", 
	      G_CALLBACK (GnomeTrayIcon::tray_popup), this);
		
	CurrentEvent = EventList.end();
	
	to = g_timeout_add (500, (gboolean (*)(void *)) (GnomeTrayIcon::TimeOut), this);

}

GnomeTrayIcon::~GnomeTrayIcon()
{
	g_source_destroy(g_main_context_find_source_by_id(NULL,to));
	g_object_unref(G_OBJECT(tray_icon));
	
	
	std::list<WokXMLTag *>::iterator iter;
	for(iter = EventList.begin(); iter != EventList.end(); iter++)
		delete *iter;
		
	EventList.clear(); // Not stricly nessesary now
}

void
GnomeTrayIcon::tray_popup (GtkWidget *widget, guint button, guint activate_time,GnomeTrayIcon *c)
{
		char buf[20];
		WokXMLTag MenuXML(NULL, "menu");
		sprintf(buf, "%d", button);
		MenuXML.AddAttr("button", buf);
		sprintf(buf, "%d", activate_time);
		MenuXML.AddAttr("time", buf);
		MenuXML.AddTag("item").AddAttr("signal", "Get Main Menu");
		MenuXML.AddTag("item").AddAttr("signal", "GetMenu");
		WokXMLTag &data = MenuXML.AddTag("data");
		c->wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);
		
}

void 
GnomeTrayIcon::tray_icon_pressed (GtkWidget * widget, GnomeTrayIcon *c)
{
	
		if(c->EventList.empty())
		{
			WokXMLTag tag(NULL, "noinfo");
			c->wls->SendSignal("GUI Window toggle", &tag);
		
			return;		
		}
		
		if (!(c->EventList.front())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().empty() )
			c->wls->SendSignal((c->EventList.front())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetAttr("name"), 
									*(c->EventList.front())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().begin());
		
		if( c->EventList.empty() )
			return;

#warning check into how this works it really seams bad
		WokXMLTag eventtag(NULL, "event");
		eventtag.AddTag(c->EventList.front());
		c->wls->SendSignal("Jabber Event Remove", &eventtag);
	
}

void
GnomeTrayIcon::MenuActivate(GtkMenuItem *menuitem,GnomeTrayIcon *data)
{
	WokXMLTag menutag(NULL,"");
	if( data->MainMenuSignals.find(GTK_WIDGET(menuitem)) != data->MainMenuSignals.end())
		data->wls->SendSignal(data->MainMenuSignals[GTK_WIDGET(menuitem)], &menutag);	
}

void
GnomeTrayIcon::ResetList()
{
	CurrentEvent = EventList.begin();
}

gboolean 
GnomeTrayIcon::TimeOut (GnomeTrayIcon * c)
{
	c->UpdateList();
	
	return TRUE;
}

void
GnomeTrayIcon::UpdateList()
{
	if( EventList.size() )
	{
		std::string image_file;
		if ( CurrentEvent == EventList.end() )
		{
			image_file = PACKAGE_DATA_DIR"/wokjab/online.png";	
			CurrentEvent = EventList.begin();
		}
		else
		{
			image_file = (*CurrentEvent)->GetAttr("icon");
			CurrentEvent++;
		}
		
		gtk_status_icon_set_from_file(tray_icon, image_file.c_str());
	}
	else
	{
		gtk_status_icon_set_from_file(tray_icon, PACKAGE_DATA_DIR"/wokjab/online.png");
	}
}

void
GnomeTrayIcon::UpdateTooltip()
{
	std::list<WokXMLTag *>::iterator iter;
	std::stringstream str;
	for(iter = EventList.begin(); iter != EventList.end(); iter++)
	{
		if ( (**iter).GetTagList("description").size() )
			str << (**iter).GetFirstTag("description").GetBody() << std::endl;
	}

	if(str.str().size())
	{
			gtk_status_icon_set_tooltip_markup(tray_icon, str.str().substr(0,str.str().size()-1).c_str());
			gtk_status_icon_set_has_tooltip(tray_icon, TRUE);
	}
	else
	{
			gtk_status_icon_set_tooltip_markup(tray_icon, presence_tip.c_str());
			gtk_status_icon_set_has_tooltip(tray_icon, TRUE);
	}
}

int
GnomeTrayIcon::Presence(WokXMLTag *tag)
{
	presence_tip = "";
	if ( !tag->GetFirstTag("presence").GetFirstTag("show").GetBody().empty() )
		presence_tip = tag->GetFirstTag("presence").GetFirstTag("show").GetBody() + " - ";
	presence_tip +="You are set to: ";
	if ( tag->GetFirstTag("presence").GetFirstTag("status").GetBody().empty() )
		presence_tip += XMLisize("<no status set>");
	else
		presence_tip += XMLisize(tag->GetFirstTag("presence").GetFirstTag("status").GetBody());
	
	if ( EventList.empty() )
	{
			gtk_status_icon_set_tooltip_markup(tray_icon, presence_tip.c_str());
			gtk_status_icon_set_has_tooltip(tray_icon, TRUE);
	}
	
	return 1;	
}

int
GnomeTrayIcon::AddJIDEvent(WokXMLTag *tag)
{
	std::list<WokXMLTag *>::iterator itemiter;
	for(itemiter = tag->GetTagList("item").begin() ; itemiter != tag->GetTagList("item").end(); itemiter++)
	{
		std::list<WokXMLTag *>::iterator iter;
		for(iter = EventList.begin(); iter != EventList.end(); iter++)
		{
			if( **iter == **itemiter )
				return 1;
		}
		
		EventList.push_front(new WokXMLTag(**itemiter));
		// Should move the event to the front if its in the list .. FIX
		
		ResetList();
		UpdateList();
		UpdateTooltip();
	}		
	return 1;
}

int
GnomeTrayIcon::RemoveJIDEvent( WokXMLTag *tag)
{
	std::list<WokXMLTag *>::iterator itemiter;
	for(itemiter = tag->GetTagList("item").begin() ; itemiter != tag->GetTagList("item").end(); itemiter++)
	{
		std::list<WokXMLTag *>::iterator iter;
		std::list<WokXMLTag *>::iterator nxtiter;
		
		for(iter = EventList.begin(); iter != EventList.end(); )
		{
			if( (*itemiter)->GetAttr("id") == (*iter)->GetAttr("id") )
			{
				nxtiter = iter;
				nxtiter++;
				
				delete *iter;
				EventList.erase(iter);
				
				iter = nxtiter;
			}
			else
				iter++;
		}
		
		ResetList();
		UpdateList();
		UpdateTooltip();
	}	
	return true;
}
