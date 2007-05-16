/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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

#include "browserwid.h"


BrowserWidget::BrowserWidget(WLSignal *wls, Browser *bro):
WLSignalInstance ( wls ), bro(bro)
{
	GtkWidget *jid_label;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkCellRenderer *renderer;
	GtkWidget *scrolledwindownode;
	
	menu_jid = "";
	menu_node = "";
	activesession = "";
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Jabber Browser");
	hbox = gtk_hbox_new(FALSE, FALSE);
	vbox = gtk_vbox_new(FALSE, FALSE);
	scrolledwindownode = gtk_scrolled_window_new (NULL, NULL);	
	
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindownode), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// Session selector 
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	sessionchooser = gtk_combo_box_new_with_model(GTK_TREE_MODEL(sessionmenu));

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (sessionchooser), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (sessionchooser), renderer,
					"text", 0,
					NULL);
	
	WokXMLTag querytag(NULL,"session");
	wls->SendSignal("Jabber GetSessions", &querytag);
	
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = querytag.GetTagList("item").begin() ; iter != querytag.GetTagList("item").end() ; iter++)
	{
		WokXMLTag querytag(NULL, "query");
		WokXMLTag &itemtag = querytag.AddTag("item");
		itemtag.AddAttr("session", (*iter)->GetAttr("name"));
		wls->SendSignal("Jabber Connection GetUserData", &querytag);
		std::string name= (*iter)->GetAttr("name") + ": " + itemtag.GetFirstTag("jid").GetBody();
		
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(sessionmenu), &treeiter);
		gtk_list_store_set(GTK_LIST_STORE(sessionmenu), &treeiter, 0 , itemtag.GetFirstTag("jid").GetBody().c_str(),
						1, (*iter)->GetAttr("name").c_str(), -1);
	}

	// Tree view ...
	nodetree = gtk_tree_view_new ();
	
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (nodetree),
        -1, "Name", renderer, "text", 0, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (nodetree),
        -1, "JID", renderer, "text", 1, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (nodetree),
        -1, "Session", renderer, "text", 2, NULL);
  
  store = gtk_tree_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	
	model = GTK_TREE_MODEL(store);
	gtk_tree_view_set_model (GTK_TREE_VIEW (nodetree), model);
	g_object_unref (model);
	// End tree view
	
	jid_label = gtk_label_new_with_mnemonic("Address:");
	button = gtk_button_new_with_label ("Browse");
	address_entry = gtk_entry_new();
	
	
	GtkTreeIter treeiter;
	if( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(sessionmenu), &treeiter) )
	{
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(sessionchooser), &treeiter);
		UpdateEntryWidget();
	}
	
	gtk_box_pack_start(GTK_BOX(hbox), jid_label, FALSE, 0,0);
	gtk_box_pack_start(GTK_BOX(hbox), address_entry, FALSE, 0,0);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, 0,0);
	gtk_box_pack_start(GTK_BOX(vbox), sessionchooser, FALSE, 0,0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox), scrolledwindownode, TRUE, TRUE ,2);
	gtk_container_add (GTK_CONTAINER (scrolledwindownode), nodetree);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (BrowserWidget::ButtonPress), this);
	g_signal_connect (G_OBJECT (window), "destroy",
					G_CALLBACK (BrowserWidget::Destroy), this);
	g_signal_connect (G_OBJECT (nodetree), "row-expanded",
					G_CALLBACK (BrowserWidget::on_nodetree_row_expanded), this);
	g_signal_connect (G_OBJECT (nodetree), "button_press_event", 
			  G_CALLBACK (BrowserWidget::popup_menu), this);
	g_signal_connect (G_OBJECT (sessionchooser), "changed", 
			  G_CALLBACK (BrowserWidget::ComboBoxChange), this);
	
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
	gtk_widget_show_all(window);
}

BrowserWidget::~BrowserWidget()
{
	bro->BrowserWidRemove(this);
	if ( window )
	{
		GtkWidget *wid = window;
		window = NULL;
		gtk_widget_destroy( wid );
	}
}

void
BrowserWidget::UpdateEntryWidget()
{
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(sessionchooser), &treeiter) )
	{
		return;
	}
		
	gchar *session;
	
	gtk_tree_model_get (GTK_TREE_MODEL(sessionmenu), &treeiter, 1, &session, -1);
	activesession = session;
	
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", session);
	wls->SendSignal("Jabber Connection GetUserData", &querytag);
	gtk_entry_set_text(GTK_ENTRY(address_entry),itemtag.GetFirstTag("server").GetBody().c_str());
	
}


void
BrowserWidget::Connect(std::string signame)
{
	EXP_SIGHOOK(signame.c_str(), &BrowserWidget::GetItemData, 500);
	iqsignalhooks.push_back(signame);
}

void
BrowserWidget::InfoConnect(std::string signame)
{
	EXP_SIGHOOK(signame.c_str(), &BrowserWidget::GetInfoData, 500);
	iqinfosignalhooks.push_back(signame);
}

void
BrowserWidget::Destroy (GtkWidget * widget, BrowserWidget *c)
{
	if ( c->window )
	{
		c->window = NULL;
		delete c;
	}
}

gboolean
BrowserWidget::popup_menu(GtkTreeView *tree_view, GdkEventButton *event, BrowserWidget *c)
{
	GtkTreePath      *path;
	if (event->button == 3 && 
		gtk_tree_view_get_path_at_pos (tree_view, (int)event->x, (int)event->y, &path, NULL, NULL, NULL)) 
	{
		GtkWidget *menu_item;
		GtkTreeIter iter;

		gtk_tree_model_get_iter (GTK_TREE_MODEL(c->model), &iter, path);
		c->pop_menu = gtk_menu_new();

		gchar *node;   
		gchar *jid;
		gchar *session;
		
		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &iter,
        1, &jid, 2, &node, 3, &session, -1);
		
		c->menu_jid = jid;
		c->menu_node = node;
		c->menu_session = session;
		
		g_free(jid);
		g_free(node);
		g_free(session);
		
		WokXMLTag msgtag(NULL,"message");
		msgtag.AddAttr("session", session);
		WokXMLTag &tag = msgtag.AddTag("iq");
		tag.AddAttr("to", jid );
		tag.AddAttr("type", "get");
		WokXMLTag *query;
		query = &tag.AddTag("query");
		query->AddAttr("xmlns", "http://jabber.org/protocol/disco#info");
		if( c->menu_node != "" )
			query->AddAttr("node", node);
	
		c->wls->SendSignal("Jabber XML IQ Send", &msgtag);
		c->InfoConnect("Jabber XML IQ ID " + tag.GetAttr("id"));
		
		
		
		menu_item = gtk_menu_item_new_with_mnemonic ("Features");
		gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (c->pop_menu), menu_item);
		
		gtk_menu_popup (GTK_MENU(c->pop_menu), NULL, NULL, NULL, NULL,
        		event->button, event->time);
		
		return true;
	}		
	return false;
}

void
BrowserWidget::on_nodetree_row_expanded       (GtkTreeView     *treeview,
                                 GtkTreeIter     *arg1,
                                 GtkTreePath     *arg2,
                                 BrowserWidget *c)
{
	gchar *node;   
	gchar *jid;
	gchar *session;
  gtk_tree_model_get(GTK_TREE_MODEL(c->model), arg1,
        2, &node,
				1, &jid, 
				3, &session, -1);
	
	if (node && jid && session)
  {
		if ( c->nodes.find(node) == c->nodes.end() )
		{
			WokXMLTag msgtag(NULL, "message");
			msgtag.AddAttr("session", session);
			WokXMLTag &tag = msgtag.AddTag("iq");
			tag.AddAttr("to", jid );
			tag.AddAttr("type", "get");
			WokXMLTag *query = &tag.AddTag("query");
			query->AddAttr("xmlns", "http://jabber.org/protocol/disco#items");
			query->AddAttr("node", node);
			c->wls->SendSignal("Jabber XML IQ Send", &msgtag);
			c->Connect("Jabber XML IQ ID " + tag.GetAttr("id"));
			c->nodes[node]=13;
		}
	}
}

gboolean
BrowserWidget::MenuActivate(GtkMenuItem *menuitem, BrowserWidget *c)
{
	g_assert( c->popupmenusignals.find(GTK_WIDGET(menuitem)) != c->popupmenusignals.end());

	WokXMLTag tag(NULL, "servicefeature");
	tag.AddAttr("jid", c->menu_jid);
	tag.AddAttr("session", c->menu_session);
	c->wls->SendSignal("Jabber disco Feature " + c->popupmenusignals[GTK_WIDGET(menuitem)], tag);
		
	return true;
}

void
BrowserWidget::ComboBoxChange(GtkComboBox *widget, BrowserWidget *c)
{
	c->UpdateEntryWidget();
}

void
BrowserWidget::ButtonPress(GtkWidget *widget, BrowserWidget *c)
{
	WokXMLTag msgtag ( NULL, "message");
	msgtag.AddAttr("session", c->activesession);
	
	WokXMLTag &tag = msgtag.AddTag("iq");
	tag.AddAttr("to", gtk_entry_get_text(GTK_ENTRY(c->address_entry)) );
	tag.AddAttr("type", "get");
	tag.AddTag("query").AddAttr("xmlns", "http://jabber.org/protocol/disco#items");
	
	c->wls->SendSignal("Jabber XML IQ Send", &msgtag);
	c->Connect("Jabber XML IQ ID " + tag.GetAttr("id"));
	
	c->nodes.clear();
}

void
BrowserWidget::AddNode(GtkTreeIter *parant, std::string jid, std::string name, std::string node, std::string session)
{
	GtkTreeIter    iter;
	
	gtk_tree_store_append (store, &iter, parant);
  gtk_tree_store_set (store, &iter,
                      0, name.c_str(),
                      1, jid.c_str(),
											2, node.c_str(),
											3, session.c_str(),
                      -1); 
	
	if ( node != "" )
	{
		GtkTreeIter subiter;
		
		gtk_tree_store_append( store, &subiter, &iter);
	}
}

int
BrowserWidget::FindTreeIter(GtkTreeIter *parant, std::string jid, std::string node, std::string session)
{
	gchar *iter_node;   
	gchar *iter_jid;
	
	for(;;)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model), parant,
        2, &iter_node, -1);
		gtk_tree_model_get(GTK_TREE_MODEL(model), parant,
        1, &iter_jid, -1);
	
		if( jid == iter_jid )
		{
			if( node == iter_node )
				return true;
			else if( gtk_tree_model_iter_has_child(model, parant))
			{
				GtkTreeIter iter = *parant;
				gtk_tree_model_iter_children    (model, &iter, parant);
				if ( FindTreeIter( &iter, jid, node, session) )
				{
					*parant = iter;
					return true;
				}
			}
		}
		else
		{
			if ( ! gtk_tree_model_iter_next(model, parant) )
			{
				parant = NULL;
				return false;
			}
		}
	}
}

int 
BrowserWidget::GetInfoData(WokXMLTag *tag)
{
	WokXMLTag *querytag;
	querytag = &tag->GetFirstTag("iq").GetFirstTag("query");
	
	if( querytag->GetAttr("node") != menu_node || tag->GetFirstTag("iq").GetAttr("from") != menu_jid )
		return true;
	
	GtkWidget *menu_item;
	std::list <WokXMLTag *> *list;
	std::list <WokXMLTag *>::iterator iter;
	popupmenusignals.clear();
	
	list = &querytag->GetTagList("feature");
	for( iter = list->begin() ; iter != list->end() ; iter++)
	{
		menu_item = gtk_menu_item_new_with_mnemonic ((*iter)->GetAttr("var").c_str());
		gtk_widget_show (menu_item);
		gtk_container_add (GTK_CONTAINER (pop_menu), menu_item);
		
		gtk_signal_connect (GTK_OBJECT (menu_item), "activate",
                      GTK_SIGNAL_FUNC (BrowserWidget::MenuActivate),
                      this);
		
		popupmenusignals[GTK_WIDGET(menu_item)] = (*iter)->GetAttr("var").c_str();
	}
	return true;
}

void
BrowserWidget::GetItemData(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> *list;
	WokXMLTag *querytag;
	querytag = &tag->GetFirstTag("iq").GetFirstTag("query");
	GtkTreeIter *parantiter = NULL;
	GtkTreeIter parant;
	GtkTreeIter itertoremove;
	bool deliter = false;
	
	if( querytag->GetAttr("node") != "" )
	{
		gtk_tree_model_get_iter_first   (model, &parant);
		FindTreeIter(&parant, querytag->GetAttr("jid"), querytag->GetAttr("node"), tag->GetAttr("session"));
		
		if( gtk_tree_model_iter_has_child(model, &parant))
		{
			gtk_tree_model_iter_children (model, &itertoremove, &parant);
			deliter = true;
		}
		parantiter = &parant;
	}
	else 
		gtk_tree_store_clear(store);
	
	list = &querytag->GetTagList("item");
	for( iter = list->begin() ; iter != list->end() ; iter++)
	{
		AddNode(parantiter, (*iter)->GetAttr("jid"), (*iter)->GetAttr("name"), (*iter)->GetAttr("node"), tag->GetAttr("session"));
	}
	
	if( deliter )
		gtk_tree_store_remove(store, &itertoremove);
}

