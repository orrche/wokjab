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
using std::cout;
using std::endl;

#include "../include/GnomeTrayIcon.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GnomeTrayIcon::GnomeTrayIcon(WLSignal *wls) : WoklibPlugin (wls)
{
	
	tray_icon 	= egg_tray_icon_new("WokJab Tray Icon");
	if ( tray_icon )
	{
		EXP_SIGHOOK("Jabber Event Add", &GnomeTrayIcon::AddJIDEvent, 1000);
		EXP_SIGHOOK("Jabber Event Remove", &GnomeTrayIcon::RemoveJIDEvent, 2);
	
		eventbox 	= gtk_event_box_new();
		tray_icon_tips = gtk_tooltips_new();

		gtk_container_add (GTK_CONTAINER (tray_icon), eventbox);

		
		image 		= gtk_image_new_from_file(PACKAGE_DATA_DIR"/wokjab/online.png");
		gtk_container_add (GTK_CONTAINER (eventbox), image);
		
		gtk_tooltips_disable(GTK_TOOLTIPS(tray_icon_tips));
		
		gtk_widget_show_all (GTK_WIDGET (tray_icon));					 
		

		g_signal_connect (G_OBJECT (eventbox), "button_press_event",
					G_CALLBACK (GnomeTrayIcon::tray_icon_pressed), this);
		CurrentEvent = EventList.end();
		
		to = g_timeout_add (500, (gboolean (*)(void *)) (GnomeTrayIcon::TimeOut), this);
	}
}

GnomeTrayIcon::~GnomeTrayIcon()
{
	if ( tray_icon )
	{
		g_source_remove(to);
	//	g_object_unref(tray_icon);
		gtk_widget_destroy(eventbox);
	}
	
	
	std::list<WokXMLTag *>::iterator iter;
	for(iter = EventList.begin(); iter != EventList.end(); iter++)
		delete *iter;
		
	EventList.clear(); // Not stricly nessesary now
}


void 
GnomeTrayIcon::tray_icon_pressed (GtkWidget * widget, GdkEventButton *event, gpointer user_data)
{
	GnomeTrayIcon *gti;
	gti = static_cast <GnomeTrayIcon*> (user_data);
	
	if(event->type == GDK_BUTTON_PRESS && event->button == 1)
	{
		if(gti->EventList.empty())
		{
			WokXMLTag tag(NULL, "noinfo");
			gti->wls->SendSignal("GUI Window toggle", &tag);
		
			return;		
		}
		
		if (!(gti->EventList.front())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().empty() )
			gti->wls->SendSignal((gti->EventList.front())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetAttr("name"), 
									*(gti->EventList.front())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().begin());
		
		if( gti->EventList.empty() )
			return;

#warning check into how this works it really seams bad
		WokXMLTag eventtag(NULL, "event");
		eventtag.AddTag(gti->EventList.front());
		gti->wls->SendSignal("Jabber Event Remove", &eventtag);
	}
	if(event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		char buf[20];
		WokXMLTag MenuXML(NULL, "menu");
		sprintf(buf, "%d", event->button);
		MenuXML.AddAttr("button", buf);
		sprintf(buf, "%d", event->time);
		MenuXML.AddAttr("time", buf);
		MenuXML.AddTag("item").AddAttr("signal", "Get Main Menu");
		MenuXML.AddTag("item").AddAttr("signal", "GetMenu");
		WokXMLTag &data = MenuXML.AddTag("data");
		gti->wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);
	}
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
		if ( CurrentEvent == EventList.end() )
		{
			gtk_image_set_from_file(GTK_IMAGE(image),  std::string(PACKAGE_DATA_DIR"/wokjab/online.png").c_str());
			
			CurrentEvent = EventList.begin();
		}
		else
		{
			gtk_image_set_from_file (GTK_IMAGE(image), (*CurrentEvent)->GetAttr("icon").c_str());
			CurrentEvent++;
		}
	}
	else
	{
		gtk_image_set_from_file(GTK_IMAGE(image),  std::string(PACKAGE_DATA_DIR"/wokjab/online.png").c_str());
	}
	
	std::list<WokXMLTag *>::iterator iter;
	std::stringstream str;
	for(iter = EventList.begin(); iter != EventList.end(); iter++)
	{
		if ( (**iter).GetTagList("description").size() )
			str << (**iter).GetFirstTag("description").GetBody() << std::endl;
	}
	if(str.str().size())
	{
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tray_icon_tips), GTK_WIDGET(eventbox),
				 		 str.str().substr(0,str.str().size()-1).c_str(),
				 		 str.str().substr(0,str.str().size()-1).c_str());
		gtk_tooltips_enable(GTK_TOOLTIPS(tray_icon_tips));
	
	}
	else
		gtk_tooltips_disable(GTK_TOOLTIPS(tray_icon_tips));
		
}

int
GnomeTrayIcon::AddJIDEvent(WokXMLTag *tag)
{
	std::cout << "Event: " << *tag << std::endl;
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
			if( (**itemiter).In(**iter) )
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
	}	
	return true;
}
