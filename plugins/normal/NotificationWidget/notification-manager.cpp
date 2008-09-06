/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "notification-manager.hpp"
#include <sstream>

NotificationManager::NotificationManager(WLSignal *wls) : WoklibPlugin(wls)
{	
	EXP_SIGHOOK("Jabber Event Add", &NotificationManager::AddJIDEvent, 1500);
	EXP_SIGHOOK("Jabber Event Remove", &NotificationManager::RemoveJIDEvent, 1000);
	
	EXP_SIGHOOK("Notification Add", &NotificationManager::Add, 1000);
	EXP_SIGHOOK("Notification Remove", &NotificationManager::Remove, 1000);
	pos = items.begin();
	
	
	EXP_SIGHOOK("GUI Window Init", &NotificationManager::GUIWindowInit, 550);
	
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/notification.control.glade", "mainbox", NULL);
	
	mainwindowplug = gtk_plug_new(0);
	
	gtk_container_add(GTK_CONTAINER(mainwindowplug), glade_xml_get_widget (gxml, "mainbox"));
	gtk_widget_show_all(mainwindowplug);
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "close")), "clicked",
			G_CALLBACK (NotificationManager::CloseButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "left")), "clicked",
			G_CALLBACK (NotificationManager::LeftButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "right")), "clicked",
			G_CALLBACK (NotificationManager::RightButton), this);
		
	char buf[200];
	WokXMLTag contag(NULL, "connect");
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(mainwindowplug)));
	WokXMLTag &widtag = contag.AddTag("widget");
	widtag.AddAttr("id", buf);
	widtag.AddAttr("expand", "false");
	widtag.AddAttr("fill", "false");
	
	if ( !wls->SendSignal("GUI Window AddWidget",&contag) )
	{
		inittag = new WokXMLTag (contag);
	}
	else
	{
		inittag = NULL;
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
	//	EXP_SIGHOOK(sig.str(), &GUIRoster::Close, 500);
	}   
	
	
	/* Test debug stuff */
#if 0
	WokXMLTag msg("message");
	WokXMLTag &item = msg.AddTag("item");
	item.AddTag("body").AddText("SL: Diablo III released");
	item.AddAttr("id", "bullshit");
	wls->SendSignal("Notification Add", msg);

	WokXMLTag msg2("message");
	item = msg2.AddTag("item");
	item.AddTag("body").AddText("Facebook: John blund wants to add you as a friend");
	item.AddAttr("id", "bullshit 2");
	
	WokXMLTag &commands = item.AddTag("commands");
	
	WokXMLTag &cmd1 = commands.AddTag("command");
	cmd1.AddAttr("name", "Befriend him");
	cmd1.AddTag("signal").AddAttr("name", "Facebook befriend");
	cmd1.GetFirstTag("signal").AddTag("shitdata").AddTag("muhaha");
	wls->SendSignal("Notification Add", msg);
#endif 
	Update();
	
}

int
NotificationManager::AddJIDEvent(WokXMLTag *tag)
{
	WokXMLTag toastertag(NULL, "toaster");
	WokXMLTag &bodytag = toastertag.AddTag("body");
	bodytag.AddText(tag->GetFirstTag("item").GetFirstTag("description").GetBody());
	toastertag.AddTag(&tag->GetFirstTag("item").GetFirstTag("commands"));
	
	WokXMLTag notification("notification");
	notification.GetFirstTag("item").AddTag("body").AddText(tag->GetFirstTag("item").GetFirstTag("description").GetBody());
	notification.GetFirstTag("item").AddTag(&tag->GetFirstTag("item").GetFirstTag("commands"));
	notification.GetFirstTag("item").AddAttr("id", "JabberEvent: " + tag->GetFirstTag("item").GetAttr("id"));
	wls->SendSignal("Notification Add", &notification);
	
	return 1;	
}

int
NotificationManager::RemoveJIDEvent(WokXMLTag *tag)
{
	WokXMLTag notification("notification");
	notification.GetFirstTag("item").AddAttr("id" , "JabberEvent: " + tag->GetFirstTag("item").GetAttr("id"));
	
	wls->SendSignal("Notification Remove", notification);
	return 1;	
}


void
NotificationManager::CloseButton(GtkWidget *widget, NotificationManager *c)
{
	if ( c->pos != c->items.end() )
	{
		WokXMLTag remove("remove");
		remove.AddTag("item").AddAttr("id", (*c->pos)->GetId());
		c->wls->SendSignal("Notification Remove", remove);
	}
}

void
NotificationManager::LeftButton(GtkWidget *widget, NotificationManager *c)
{
	if ( c->pos != c->items.begin() )
	{
		gtk_widget_hide((*c->pos)->GetWidget());
		c->pos--;
		c->Update();		
	}
			
}

void
NotificationManager::RightButton(GtkWidget *widget, NotificationManager *c)
{
	std::list <NotificationWidget*>::iterator tmp;
	tmp = c->pos;
	tmp++;
	if ( tmp != c->items.end() )
	{
		gtk_widget_hide((*c->pos)->GetWidget());
		c->pos = tmp;
		c->Update();
		
	}
}

int
NotificationManager::GUIWindowInit(WokXMLTag *tag)
{
	if( inittag )
	{
		wls->SendSignal("GUI Window AddWidget",inittag);
		delete inittag;
		inittag = NULL;
		
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
//		EXP_SIGHOOK(sig.str(), &GUIRoster::Close, 500);
	}
	return 1;
}

void
NotificationManager::Update()
{
	if ( items.empty() )
	{
		gtk_widget_hide(glade_xml_get_widget (gxml, "mainbox"));
		return;
	}
	
	gtk_widget_show(glade_xml_get_widget (gxml, "mainbox"));
	gtk_widget_show_all((*pos)->GetWidget());
	
	
	if ( pos == items.begin() )
		gtk_widget_set_sensitive(glade_xml_get_widget (gxml, "left"), FALSE);
	else
		gtk_widget_set_sensitive (glade_xml_get_widget (gxml, "left"), TRUE);
	std::list <NotificationWidget *>::iterator tmp = pos;
	tmp++;
	if ( tmp == items.end() )
		gtk_widget_set_sensitive(glade_xml_get_widget (gxml, "right"), FALSE);
	else
		gtk_widget_set_sensitive(glade_xml_get_widget (gxml, "right"), TRUE);
}

int
NotificationManager::Add(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("id").empty()) 
		{
			std::stringstream str;
			str << id++;
		
			(*iter)->AddAttr("id", "NotificationManager " + str.str());
		}
		NotificationWidget *tmpwid = new NotificationWidget(wls, *iter);
		items.push_back(tmpwid); 
		gtk_box_pack_start(GTK_BOX(glade_xml_get_widget (gxml, "mainbox")), tmpwid->GetWidget(), FALSE, FALSE, 0);
	}
	
	if ( pos == items.end() )
		pos = items.begin();
	Update();
	
	return 1;
}


int
NotificationManager::Remove(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("id") != "") 
		{
			if ( (*pos)->GetId() == (*iter)->GetAttr("id") )
			{
				std::list <NotificationWidget*>::iterator tmp = pos;
				pos++;
				gtk_widget_destroy((*tmp)->GetWidget());
				delete *tmp;
				items.erase(tmp);
				
				if ( !items.empty() )
				{
					if ( pos == items.end() )
					{
						pos--;
					}
				}
				
			}
			else
			{
				std::list <NotificationWidget*>::iterator siter;
				for (siter = items.begin(); siter != items.end() ; )
				{
					if ((*siter)->GetId() == (*iter)->GetAttr("id"))
					{
						std::list <NotificationWidget*>::iterator tmpiter = siter;
						tmpiter++;
						delete *siter;
						items.erase(siter);
						siter = tmpiter;
						continue;
					}
					else
						siter++;
				}						
			}
			wls->SendSignal("Jabber Notification Remove '" + XMLisize((*iter)->GetAttr("id")) + "'", (*iter));
			
		}
	}
	
	
	Update();
	return 1;
}
