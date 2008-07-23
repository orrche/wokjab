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

#include "wokjab-dock-window-handler.hpp"

WokjabDockWindowHandler::WokjabDockWindowHandler(WLSignal *wls) : WoklibPlugin(wls)
{
	WokXMLTag wargs("args");
	wls->SendSignal("Wokjab GetArgs", wargs);
	
	bool wdw_debug = false;
	std::list < WokXMLTag *>::iterator argiter;

	for ( argiter = wargs.GetFirstTag("args").GetTagList("singlearg").begin() ; argiter != wargs.GetFirstTag("args").GetTagList("singlearg").end() ; argiter++)
	{
		if ( (*argiter)->GetAttr("data") == "d" )
			wdw_debug = true;
	}

	if ( wdw_debug ) 
	{	
		GtkWidget *win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		
		/* create the dock */
		dock = gdl_dock_new ();
		GdlDockMaster *master = GDL_DOCK_OBJECT_GET_MASTER (dock);
		g_object_set (master, "switcher-style", GDL_SWITCHER_STYLE_TABS, NULL);
		
		GtkWidget *box = gtk_hbox_new( FALSE, 2);
		
		/* ... and the layout manager */
		GdlDockLayout *layout = gdl_dock_layout_new (GDL_DOCK (dock));

		/* create the dockbar */
		GtkWidget *dockbar = gdl_dock_bar_new (GDL_DOCK (dock));
		gdl_dock_bar_set_style(GDL_DOCK_BAR(dockbar), GDL_DOCK_BAR_TEXT);

		gtk_container_add(GTK_CONTAINER(win), box);
		//gtk_box_pack_start(GTK_BOX(box), dockbar, FALSE, FALSE, 0);
		gtk_box_pack_end(GTK_BOX(box), dock, TRUE, TRUE, 0);
		
		gtk_widget_show_all(dockbar);
		gtk_widget_show_all(box);
		gtk_widget_hide(win);
		
		//gdl_dock_layout_run_manager (layout);
		EXP_SIGHOOK("Wokjab DockWindow Add", &WokjabDockWindowHandler::Add, 1000);
		EXP_SIGHOOK("Wokjab DockWindow Destroy", &WokjabDockWindowHandler::Destroy, 1000);
		EXP_SIGHOOK("Wokjab DockWindow Activate", &WokjabDockWindowHandler::Activate, 1000);
		EXP_SIGHOOK("Wokjab DockWindow SetUrgencyHint", &WokjabDockWindowHandler::SetUrgency, 1000);
		
		EXP_SIGHOOK("Wokjab DockWindow Hide", &WokjabDockWindowHandler::Hide, 1000);
		EXP_SIGHOOK("Wokjab DockWindow Show", &WokjabDockWindowHandler::Show, 1000);
		
		
		EXP_SIGHOOK("GUI WindowDock AddWidget", &WokjabDockWindowHandler::AddChat, 900);
		EXP_SIGHOOK("GUI WindowDock Activate", &WokjabDockWindowHandler::ActivateChat, 900);
		EXP_SIGHOOK("GUI WindowDock HideWidget", &WokjabDockWindowHandler::HideChat, 900);
		EXP_SIGHOOK("GUI WindowDock ShowWidget", &WokjabDockWindowHandler::ShowChat, 900);
		
//		EXP_SIGHOOK("GUI Window AddWidget", &WokjabDockWindowHandler::AddRoster, 900);
	}
}

#include <sstream>
int
WokjabDockWindowHandler::AddRoster(WokXMLTag *tag)
{
	static int i = 0;
	std::stringstream str;
	str << "roster";
	if ( i++ < 100 )
	{
		tag->AddAttr("id", tag->GetFirstTag("widget").GetAttr("id"));
		if ( tag->GetAttr("identifier").empty() )
			tag->AddAttr("identifier", "roster item #" + tag->GetAttr("id"));
		tag->AddAttr("type", str.str());
		tag->AddAttr("handle", "false");
		tag->AddAttr("placement", "bottom");
		wls->SendSignal("Wokjab DockWindow Add", tag);

		return 0;
	}
	return 1;
}

int
WokjabDockWindowHandler::SetUrgency(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->SetUrgencyHint(tag);
	}
	
	return 1;
}


int
WokjabDockWindowHandler::Hide(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->Hide(tag);
	}
	return 1;
}

int 
WokjabDockWindowHandler::HideChat(WokXMLTag *tag)
{
	tag->AddAttr("id", tag->GetAttr("widget"));
	wls->SendSignal("Wokjab DockWindow Hide",tag);
	std::cout << ":::" << *tag << std::endl;
	return 0;
}


int
WokjabDockWindowHandler::Show(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->Show(tag);
	}
	return 1;
}

int 
WokjabDockWindowHandler::ShowChat(WokXMLTag *tag)
{
	tag->AddAttr("id", tag->GetAttr("widget"));
	wls->SendSignal("Wokjab DockWindow Show",tag);
	return 0;
}

int
WokjabDockWindowHandler::Activate(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->Activate();
	}
	return 1;
}

int 
WokjabDockWindowHandler::ActivateChat(WokXMLTag *tag)
{
	wls->SendSignal("Wokjab DockWindow Activate",tag);
	return 0;
}

int
WokjabDockWindowHandler::AddChat(WokXMLTag *tag)
{
	tag->AddAttr("id", tag->GetAttr("mainwidget"));	
	tag->AddAttr("labelid", tag->GetAttr("labelwidget"));
	tag->AddAttr("type", "chat");
	tag->AddAttr("handle", "true");
	tag->AddAttr("placement", "center");
	wls->SendSignal("Wokjab DockWindow Add", tag);
	
	return 0;
}

int
WokjabDockWindowHandler::Add(WokXMLTag *tag)
{
	std::map<std::string, WokjabDockWindow* >::iterator iter;
	
	for( iter = windows.begin() ; iter != windows.end() ; iter++ )
	{
		if ( iter->second->GetType() == tag->GetAttr("type") )
		{
			windows[tag->GetAttr("id")] = new WokjabDockWindow(wls, tag, dock, iter->second);	
			break;
		}
	}
	if ( iter == windows.end() )
	{
		windows[tag->GetAttr("id")] = new WokjabDockWindow(wls, tag, dock, NULL);
	}
		
	return 1;
}

int
WokjabDockWindowHandler::Destroy(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		WokjabDockWindow *tmp = windows[tag->GetAttr("id")];
		windows.erase(tag->GetAttr("id"));
		delete tmp;
	}
	
	return 1;
}
