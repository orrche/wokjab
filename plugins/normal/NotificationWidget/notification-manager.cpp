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
#include <algorithm>

NotificationManager::NotificationManager(WLSignal *wls) : WoklibPlugin(wls)
{	
	EXP_SIGHOOK("Jabber Event Add", &NotificationManager::AddJIDEvent, 1500);
	EXP_SIGHOOK("Jabber Event Remove", &NotificationManager::RemoveJIDEvent, 1000);
	
	EXP_SIGHOOK("Notification Add", &NotificationManager::Add, 1000);
	EXP_SIGHOOK("Notification Remove", &NotificationManager::Remove, 1000);
	
	EXP_SIGHOOK("GUI Window Init", &NotificationManager::GUIWindowInit, 550);
	
	removetag = NULL;
	pos = items.begin();
	
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/notification.control.glade", "mainbox", NULL);
	gxml_list = glade_xml_new(PACKAGE_GLADE_DIR"/wokjab/notification.control.glade", "list_window", NULL);
	
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer = gtk_cell_renderer_text_new ();
		
	
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "markup", 0);
	gtk_tree_view_column_set_sort_column_id (column, 0);
	gtk_tree_view_column_set_title(column,"Name");
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml_list,"event_list")), GTK_TREE_VIEW_COLUMN (column));		
		
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 1);
	gtk_tree_view_column_set_sort_column_id (column, 1);
	gtk_tree_view_column_set_title(column,"id");
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml_list,"event_list")), GTK_TREE_VIEW_COLUMN (column));		

	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 2);
	gtk_tree_view_column_set_sort_column_id (column, 2);
	gtk_tree_view_column_set_title(column,"xml");
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml_list,"event_list")), GTK_TREE_VIEW_COLUMN (column));		

	
    GtkTreeSelection *selection = GTK_TREE_SELECTION(gtk_tree_view_get_selection (GTK_TREE_VIEW (glade_xml_get_widget(gxml_list,"event_list"))));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	
	event_store = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (glade_xml_get_widget(gxml_list,"event_list")), GTK_TREE_MODEL(event_store));
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml_list, "remove")), "clicked",
			G_CALLBACK (NotificationManager::ListRemoveButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml_list, "default")), "clicked",
			G_CALLBACK (NotificationManager::ListDefaultButton), this);
	gtk_signal_connect (GTK_OBJECT (glade_xml_get_widget (gxml_list, "list_window")), "delete_event",
		GTK_SIGNAL_FUNC (gtk_widget_hide_on_delete), this);
	g_signal_connect (G_OBJECT (selection), "changed",
			G_CALLBACK (NotificationManager::ListSelectionChange), this);
	
	
	
	
	mainwindowplug = gtk_plug_new(0);
	
	gtk_container_add(GTK_CONTAINER(mainwindowplug), glade_xml_get_widget (gxml, "mainbox"));
	gtk_widget_show_all(mainwindowplug);
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "close")), "clicked",
			G_CALLBACK (NotificationManager::CloseButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "left")), "clicked",
			G_CALLBACK (NotificationManager::LeftButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "right")), "clicked",
			G_CALLBACK (NotificationManager::RightButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "list")), "clicked",
			G_CALLBACK (NotificationManager::ListButton), this);
	
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
#if 1
	WokXMLTag msg("message"); 
	WokXMLTag &item = msg.AddTag("item");
	item.AddTag("body").AddText("SL: Diablo III released");
	item.AddAttr("id", "bullshit");
	wls->SendSignal("Notification Add", msg);

	WokXMLTag msg2("message");
	item = msg2.AddTag("item");
	item.AddTag("body").AddText("Facebook: ");
	item.GetFirstTag("body").AddTag("b").AddText("John");
	item.GetFirstTag("body").AddText("blund wants to add you as a friend");
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
NotificationManager::SelectedRemove(GtkTreePath *path, NotificationManager *c)
{
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(c->event_store), &iter,path);
	
	gchar *id;
	gtk_tree_model_get(GTK_TREE_MODEL(c->event_store), &iter, 1, &id, -1);

	c->removetag->AddTag("item").AddAttr("id", id);
	
	
	g_free(id);
	
}

void
NotificationManager::ListRemoveButton(GtkWidget *widget, NotificationManager *c)
{
	GtkTreeSelection *selection;
	GList *list;
	GtkTreeModel *model;
	c->removetag = new WokXMLTag("remove");
		
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (glade_xml_get_widget(c->gxml_list,"event_list")));
	model = GTK_TREE_MODEL(c->event_store);
	list = gtk_tree_selection_get_selected_rows(selection, &model);
	g_list_foreach(list, (GFunc)(NotificationManager::SelectedRemove), c);
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);

	c->wls->SendSignal("Notification Remove", c->removetag);
	delete c->removetag;
}

void
NotificationManager::SelectedDefault(GtkTreePath *path, NotificationManager *c)
{
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(c->event_store), &iter,path);
	
	gchar *xml;
	gtk_tree_model_get(GTK_TREE_MODEL(c->event_store), &iter, 2, &xml, -1);

	WokXMLTag tag("data");
	tag.Add(xml);
	WokXMLTag &command = tag.GetFirstTag("item").GetFirstTag("commands").GetFirstTag("command");
	if ( !command.GetFirstTag("signal").GetTags().empty() )
	{
		c->wls->SendSignal(command.GetFirstTag("signal").GetAttr("name"), **command.GetFirstTag("signal").GetTags().begin());
	}
	
	g_free(xml);
}

void
NotificationManager::ListDefaultButton(GtkWidget *widget, NotificationManager *c)
{
	GtkTreeSelection *selection;
	GList *list;
	GtkTreeModel *model;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (glade_xml_get_widget(c->gxml_list,"event_list")));
	model = GTK_TREE_MODEL(c->event_store);
	list = gtk_tree_selection_get_selected_rows(selection, &model);
	g_list_foreach(list, (GFunc)(NotificationManager::SelectedDefault), c);
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);
}

void
NotificationManager::ListSelectionChange(GtkTreeView *tree_view, NotificationManager *c) 
{
	GtkTreeSelection *selection;
	GList *list;
	GtkTreeModel *model;
	gtk_container_foreach (GTK_CONTAINER(glade_xml_get_widget(c->gxml_list, "buttonBox")),  (void(*)(GtkWidget *, void *))gtk_widget_destroy, NULL);

	std::list <std::string> composesig;	
	
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (glade_xml_get_widget(c->gxml_list,"event_list")));
	model = GTK_TREE_MODEL(c->event_store);
	list = gtk_tree_selection_get_selected_rows(selection, &model);
	for ( GList *pos = list ; pos ; pos = pos->next)
	{
		GtkTreeIter iter;
		gtk_tree_model_get_iter(GTK_TREE_MODEL(c->event_store), &iter,(GtkTreePath*)pos->data);
	
		gchar *xml;
		gtk_tree_model_get(GTK_TREE_MODEL(c->event_store), &iter, 2, &xml, -1);

		WokXMLTag tag("data");
		tag.Add(xml);
		WokXMLTag &command = tag.GetFirstTag("item").GetFirstTag("commands");
		for( std::list <WokXMLTag *>::iterator liter = command.GetTagList("command").begin(); liter != command.GetTagList("command").end(); liter++)
		{
			if ( std::find(composesig.begin(), composesig.end(), (*liter)->GetAttr("name")) == composesig.end() )
			{
				composesig.push_back((*liter)->GetAttr("name"));
				GtkWidget *button = gtk_button_new_with_label((*liter)->GetAttr("name").c_str());
				gtk_box_pack_start(GTK_BOX(glade_xml_get_widget(c->gxml_list, "buttonBox")), button, FALSE, FALSE, 0);

				g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (NotificationManager::SigButton), c);		
			}
		
		}
	
		g_free(xml);
	}
	
	
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);
	
	
	gtk_widget_show_all(glade_xml_get_widget(c->gxml_list, "buttonBox"));
}

void
NotificationManager::SigButton(GtkWidget *button, NotificationManager *c)
{
	GtkTreeSelection *selection;
	GList *list;
	GtkTreeModel *model;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (glade_xml_get_widget(c->gxml_list,"event_list")));
	model = GTK_TREE_MODEL(c->event_store);
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	for ( GList *pos = list ; pos ; pos = pos->next)
	{
		gchar *xml;
		GtkTreeIter iter;
		gtk_tree_model_get_iter(GTK_TREE_MODEL(c->event_store), &iter,(GtkTreePath*)(pos->data));
		gtk_tree_model_get(GTK_TREE_MODEL(c->event_store), &iter, 2, &xml, -1);

		WokXMLTag tag("data");
		tag.Add(xml);
		WokXMLTag &command = tag.GetFirstTag("item").GetFirstTag("commands");
		for( std::list <WokXMLTag *>::iterator liter = command.GetTagList("command").begin(); liter != command.GetTagList("command").end(); liter++)
		{
			if((*liter)->GetAttr("name") == gtk_button_get_label(GTK_BUTTON(button)))
			{
				c->wls->SendSignal((*liter)->GetFirstTag("signal").GetAttr("name"), **(*liter)->GetFirstTag("signal").GetTags().begin());
				
			}
		}
	
		g_free(xml);
	}
	
	
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (list);
	
}


void
NotificationManager::ListButton(GtkWidget *widget, NotificationManager *c)
{
	gtk_window_present(GTK_WINDOW(glade_xml_get_widget(c->gxml_list, "list_window")));
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
	
	
	std::list<NotificationWidget *>::iterator p = items.begin();
	int i = 1;
	for( ; p != pos ; i++, p++);
	std::stringstream lpos;
	lpos << i << "/" << items.size();
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(gxml, "position_label")), lpos.str().c_str());
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
		
		
		// Adding to the list widget
		GtkTreeIter titer;
					
		gtk_list_store_append (event_store, &titer);
		gtk_list_store_set (event_store, &titer, 
									0, (*iter)->GetFirstTag("body").GetChildrenStr().c_str(),
									1, (*iter)->GetAttr("id").c_str(), 
									2, ((*iter)->GetStr()).c_str(), 
									3, ((*iter)->GetFirstTag("body").GetChildrenStr() + "\n\n-------\n\n" + XMLisize((*iter)->GetStr())).c_str(), -1 );
		
		
		

		
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
			// Searching throw all the rows for the one with the correct id 
			GtkTreeIter titer;
			gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(event_store), &titer);
			while(valid != FALSE)
			{
				gchar *id;
				
				gtk_tree_model_get(GTK_TREE_MODEL(event_store), &titer, 1, &id, -1);

				if ((*iter)->GetAttr("id")==id)
					valid =  gtk_list_store_remove (event_store, &titer);
				else
					valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(event_store), &titer);
				
				g_free(id);
			}
			
			if ( pos != items.end() && (*pos)->GetId() == (*iter)->GetAttr("id") )
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
					siter++;
				}						
			}
		
			wls->SendSignal("Jabber Notification Remove '" + XMLisize((*iter)->GetAttr("id")) + "'", (*iter));
			
		}
	}
	
	
	Update();
	return 1;
}
