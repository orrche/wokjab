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

#include "pub-sub-widget.hpp"
#include <sstream>

PubSub_Widget::PubSub_Widget(WLSignal *wls, PubSubManager *parant) : WLSignalInstance(wls), parant(parant)
{
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /PubSub_Manager/window", &PubSub_Widget::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/PubSub_Manager/window");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/pubsub.glade", NULL, NULL);
		
	GtkWidget *sessionchooser;
	GtkCellRenderer *renderer;
	
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	jidmenu = gtk_list_store_new(1, G_TYPE_STRING);
	nodemenu = gtk_list_store_new(1, G_TYPE_STRING);
	affiliationlist = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
										 
	sessionchooser = glade_xml_get_widget(xml, "session");
	gtk_combo_box_set_model (GTK_COMBO_BOX(sessionchooser), GTK_TREE_MODEL(sessionmenu) );
	
	GtkWidget *jidchooser = glade_xml_get_widget(xml, "box_jid");
	gtk_combo_box_set_model (GTK_COMBO_BOX(jidchooser), GTK_TREE_MODEL(jidmenu) );
	gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY(jidchooser), 0);
	
	GtkWidget *jidchooser2 = glade_xml_get_widget(xml, "box_jid2");
	gtk_combo_box_set_model (GTK_COMBO_BOX(jidchooser2), GTK_TREE_MODEL(jidmenu) );
	gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY(jidchooser2), 0);
	
	GtkWidget *nodechooser = glade_xml_get_widget(xml, "box_node");
	gtk_combo_box_set_model (GTK_COMBO_BOX(nodechooser), GTK_TREE_MODEL(nodemenu) );
	gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY(nodechooser), 0);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (sessionchooser), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (sessionchooser), renderer,
					"text", 0,
					NULL);
	
	GtkWidget *affiliationwid = glade_xml_get_widget(xml, "affiliationlist");
	gtk_tree_view_set_model(GTK_TREE_VIEW(affiliationwid), GTK_TREE_MODEL(affiliationlist));
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, NULL);
	g_signal_connect (renderer, "edited", G_CALLBACK (PubSub_Widget::cell_edited), this);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (affiliationwid),
        -1, "JID", renderer, "text", 0, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (affiliationwid),
        -1, "Affiliation", renderer, "text", 1, NULL);
	
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
	
	g_signal_connect (G_OBJECT (sessionchooser), "changed", 
		  	G_CALLBACK (PubSub_Widget::SessionChange), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "window")), "delete-event",
			G_CALLBACK (PubSub_Widget::Delete), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "find_jid")), "clicked", 
			G_CALLBACK (PubSub_Widget::FindJidButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "find_jid2")), "clicked", 
			G_CALLBACK (PubSub_Widget::FindJidButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "find_node")), "clicked", 
			G_CALLBACK (PubSub_Widget::FindNodeButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "button_config")), "clicked", 
			G_CALLBACK (PubSub_Widget::ConfigButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "button_register")), "clicked", 
			G_CALLBACK (PubSub_Widget::RegisterButton), this);	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "button_read")), "clicked", 
			G_CALLBACK (PubSub_Widget::ReadButton), this);	
	
	gtk_window_set_default_size(GTK_WINDOW(glade_xml_get_widget(xml,"window")), 
				atoi(config->GetFirstTag("size").GetAttr("width").c_str()),
				atoi(config->GetFirstTag("size").GetAttr("height").c_str()));
	
	gtk_widget_show_all(glade_xml_get_widget(xml,"window"));	
}

PubSub_Widget::~PubSub_Widget()
{
	GtkWidget *window = glade_xml_get_widget(xml,"window");
	int width, height;
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);

	std::stringstream s_width, s_height;
	s_width << width;
	s_height << height;

	config->GetFirstTag("size").AddAttr("width", s_width.str().c_str());
	config->GetFirstTag("size").AddAttr("height", s_height.str().c_str());

	SaveConfig();
	
	g_object_unref(xml);
	
	gtk_widget_destroy(window);
}

void
PubSub_Widget::cell_edited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, PubSub_Widget *c)
{
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	//gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
	gtk_tree_model_get_iter (GTK_TREE_MODEL(c->affiliationlist), &iter, path);

	gchar *old_text;

	gtk_tree_model_get (GTK_TREE_MODEL(c->affiliationlist), &iter, 0, &old_text, -1);
	
	/*
	GtkTreeIter useriter;
	
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(c->inlist), &useriter) )
	{
		do
		{
			gchar *jid;
			gtk_tree_model_get(GTK_TREE_MODEL(c->inlist), &useriter, 0, &jid, -1);
			WokXMLTag remove_tag("remove_tag");
			remove_tag.AddAttr("jid", jid);
			remove_tag.AddAttr("group", old_text);
			remove_tag.AddAttr("session", c->selected_session);
			c->wls->SendSignal("Roster Remove User From Group", remove_tag);
			g_free(jid);
		}
		while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(c->inlist), &useriter) );
	}
	
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(c->inlist), &useriter) )
	{
		do
		{
			gchar *jid;
			gtk_tree_model_get(GTK_TREE_MODEL(c->inlist), &useriter, 0, &jid, -1);
			WokXMLTag add_tag("add_tag");
			add_tag.AddAttr("jid", jid);
			add_tag.AddAttr("group", new_text);
			add_tag.AddAttr("session", c->selected_session);
			c->wls->SendSignal("Roster Add User To Group", add_tag);
			g_free(jid);
		}
		while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(c->inlist), &useriter) );
	}
	*/
	
	g_free (old_text);

	gtk_list_store_set (GTK_LIST_STORE (c->affiliationlist), &iter, 0, new_text, -1);

	gtk_tree_path_free (path);
}

gboolean
PubSub_Widget::Delete( GtkWidget *widget, GdkEvent *event, PubSub_Widget *c)
{
	c->parant->DialogOpenerRemove(c);
	return TRUE;
}

int 
PubSub_Widget::ConfigIQResp(WokXMLTag *tag)
{
	wls->SendSignal("Jabber jabber:x:data Init", tag->GetFirstTag("iq").GetFirstTag("pubsub").GetFirstTag("configure"));
	
	return 1;	
}

void
PubSub_Widget::ConfButton()
{
	const gchar *jid, *node;
	
	if ( selected_session.empty() )
	{
		woklib_message(wls, "You need to select a session first");
		return;
	}
	if ( (jid = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml, "entry_jid"))))[0] == 0 )
	{
		woklib_message(wls, "You need to select a jid first");
		return;
	}
	if ( (node = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml, "entry_node"))))[0] == 0 )
	{
		woklib_message(wls, "You need to select a node first");
		return;
	}
	
	
	WokXMLTag mesgtag("message");
	mesgtag.AddAttr("session", selected_session);
	WokXMLTag &iq = mesgtag.AddTag("iq");
	iq.AddAttr("type", "get");
	iq.AddAttr("to", jid);
	WokXMLTag &pubsub = iq.AddTag("pubsub");
	pubsub.AddAttr("xmlns", "http://jabber.org/protocol/pubsub#owner");
	pubsub.AddTag("configure").AddAttr("node", node);
	
	wls->SendSignal("Jabber XML IQ Send", mesgtag);
	
	EXP_SIGHOOK("Jabber XML IQ ID " + mesgtag.GetFirstTag("iq").GetAttr("id"), &PubSub_Widget::ConfigIQResp, 1000);
	/*
	<iq type='get'
		from='hamlet@denmark.lit/elsinore'
		to='pubsub.shakespeare.lit'
		id='config1'>
	  <pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>
		<configure node='princely_musings'/>
	  </pubsub>
	</iq>
	*/
}

int
PubSub_Widget::RegisterIQResp(WokXMLTag *tag)
{
	if( tag->GetFirstTag("iq").GetAttr("type") == "result")
	{
		woklib_message(wls, "PubSub Registration Sucessfull");
		return 1;
	}
	else if ( tag->GetFirstTag("iq").GetAttr("type") == "error") 
	{
		if ( !tag->GetFirstTag("iq").GetFirstTag("error").GetTagList("conflict").empty() )
			woklib_error(wls, "PubSub node most likely already registered");
		else
			woklib_error(wls, "PubSub registration failed");
		return 1;		
	}
	
	return 1;	
}

void
PubSub_Widget::RegButton()
{
	const gchar *jid, *node;
	
	if ( selected_session.empty() )
	{
		woklib_message(wls, "You need to select a session first");
		return;
	}
	if ( (jid = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml, "entry_jid"))))[0] == 0 )
	{
		woklib_message(wls, "You need to select a jid first");
		return;
	}
	if ( (node = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml, "entry_node"))))[0] == 0 )
	{
		woklib_message(wls, "You need to select a node first");
		return;
	}
	
	WokXMLTag mesgtag("message");
	mesgtag.AddAttr("session", selected_session);
	WokXMLTag &iq = mesgtag.AddTag("iq");
	iq.AddAttr("to", jid);
	iq.AddAttr("type", "set");
	WokXMLTag &pubsub = iq.AddTag("pubsub");
	pubsub.AddAttr("xmlns", "http://jabber.org/protocol/pubsub");
	pubsub.AddTag("create").AddAttr("node", node);
	pubsub.AddTag("configure");
	
	wls->SendSignal("Jabber XML IQ Send", mesgtag);
	
	EXP_SIGHOOK("Jabber XML IQ ID " + mesgtag.GetFirstTag("iq").GetAttr("id"), &PubSub_Widget::RegisterIQResp, 1000);
	/*
	<iq from='juliet@capulet.com/balcony' type='set' id='create-open'>
	  <pubsub xmlns='http://jabber.org/protocol/pubsub'>
		<create node='http://jabber.org/protocol/tune'/>
		<configure>
		</configure>
	  </pubsub>
	</iq>
	*/
}

int
PubSub_Widget::ReadIQResp(WokXMLTag *tag)
{
	std::list <WokXMLTag *> *list;
	list = NULL;
	
	std::list <WokXMLTag *>::iterator pubsubiter;
	for( pubsubiter = tag->GetFirstTag("iq").GetTagList("pubsub").begin() ; pubsubiter != tag->GetFirstTag("iq").GetTagList("pubsub").end(); pubsubiter++)
	{
		std::cout << "Pubsub" << std::endl;
		if( (*pubsubiter)->GetAttr("xmlns") == "http://jabber.org/protocol/pubsub#owner")
		{
			std::cout << "XMLNS Right" << std::endl;
			
			std::list <WokXMLTag *>::iterator afliter;
			for ( afliter = (*pubsubiter)->GetFirstTag("affiliations").GetTagList("affiliation").begin() ; afliter != (*pubsubiter)->GetFirstTag("affiliations").GetTagList("affiliation").end() ; afliter++)
			{
				std::cout << "AFLI" << std::endl;
				GtkTreeIter tIter;
				gtk_list_store_append(affiliationlist, &tIter);
				gtk_list_store_set (affiliationlist, &tIter,
                    0, (*afliter)->GetAttr("jid").c_str(),
                    1, (*afliter)->GetAttr("affiliation").c_str(),
                    -1); 
			}
										 
		}
	}
	
	/*
	<iq from='nedo@jabber.se' to='nedo@jabber.se/wokjab.test' id='wokjab7' type='result'>
		<pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>
			<affiliations node='http://jabber.org/protocol/tune'>
				<affiliation jid='nedo@jabber.se' affiliation='owner'/>
			</affiliations>
		</pubsub>
	</iq>
	*/

	return 1;
}

void
PubSub_Widget::ReadBtn()
{
	const gchar *jid, *node;
	
	if ( selected_session.empty() )
	{
		woklib_message(wls, "You need to select a session first");
		return;
	}
	if ( (jid = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml, "entry_jid"))))[0] == 0 )
	{
		woklib_message(wls, "You need to select a jid first");
		return;
	}
	if ( (node = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml, "entry_node"))))[0] == 0 )
	{
		woklib_message(wls, "You need to select a node first");
		return;
	}
	
	WokXMLTag mesgtag("message");
	mesgtag.AddAttr("session", selected_session);
	WokXMLTag &iq = mesgtag.AddTag("iq");
	iq.AddAttr("to", jid);
	iq.AddAttr("type", "get");
	WokXMLTag &pubsub = iq.AddTag("pubsub");
	pubsub.AddAttr("xmlns", "http://jabber.org/protocol/pubsub#owner");
	pubsub.AddTag("affiliations").AddAttr("node", node);
	
	wls->SendSignal("Jabber XML IQ Send", mesgtag);
	
	EXP_SIGHOOK("Jabber XML IQ ID " + mesgtag.GetFirstTag("iq").GetAttr("id"), &PubSub_Widget::ReadIQResp, 1000);
	
	/*
	<iq type='get'
		from='hamlet@denmark.lit/elsinore'
		to='pubsub.shakespeare.lit'
		id='ent1'>
	  <pubsub xmlns='http://jabber.org/protocol/pubsub#owner'>
		<affiliations node='princely_musings'/>
	  </pubsub>
	</iq>
    */
}

void
PubSub_Widget::ReadButton(GtkButton *button, PubSub_Widget *c)
{
	c->ReadBtn();
}

void
PubSub_Widget::RegisterButton(GtkButton *button, PubSub_Widget *c)
{
	c->RegButton();	
}

void 
PubSub_Widget::ConfigButton(GtkButton *button, PubSub_Widget *c)
{
	c->ConfButton();
}


void 
PubSub_Widget::FindJidButton(GtkButton *button, PubSub_Widget *c)
{
	gtk_list_store_clear(c->jidmenu);
	
	if ( c->selected_session.empty() )
	{
		woklib_message(c->wls, "You need to select a session first");
		return;
	}
	WokXMLTag pubsubjids("pusbsubjids");
	pubsubjids.AddAttr("session", c->selected_session);
	c->wls->SendSignal("Jabber PubSub Registration GetJIDs", pubsubjids);
	
	std::list <WokXMLTag *>::iterator iter;
	for ( iter = pubsubjids.GetTagList("item").begin() ; iter != pubsubjids.GetTagList("item").end() ; iter++)
	{
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(c->jidmenu), &treeiter);
		gtk_list_store_set(GTK_LIST_STORE(c->jidmenu), &treeiter, 0 , (*iter)->GetAttr("jid").c_str(), -1);
	}
}

void 
PubSub_Widget::FindNodeButton(GtkButton *button, PubSub_Widget *c)
{
	gtk_list_store_clear(c->jidmenu);
	const gchar *jid;
	
	if ( c->selected_session.empty() )
	{
		woklib_message(c->wls, "You need to select a session first");
		return;
	}
	if ( (jid = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(c->xml, "entry_jid"))))[0] == 0 )
	{
		woklib_message(c->wls, "You need to select a jid first");
		return;
	}
	
	WokXMLTag pubsubnodes("pusbsubnodess");
	pubsubnodes.AddAttr("session", c->selected_session);
	pubsubnodes.AddAttr("jid", jid);
	c->wls->SendSignal("Jabber PubSub Registration GetNodes", pubsubnodes);
	
	std::list <WokXMLTag *>::iterator iter;
	for ( iter = pubsubnodes.GetTagList("item").begin() ; iter != pubsubnodes.GetTagList("item").end() ; iter++)
	{
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(c->nodemenu), &treeiter);
		gtk_list_store_set(GTK_LIST_STORE(c->nodemenu), &treeiter, 0 , (*iter)->GetAttr("node").c_str(), -1);
	}
}

void
PubSub_Widget::SaveConfig()
{
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/PubSub_Manager/window");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /PubSub_Manager/window", &PubSub_Widget::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /PubSub_Manager/window", &PubSub_Widget::ReadConfig, 500);
}

int
PubSub_Widget::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}

void 
PubSub_Widget::SesChange()
{
	
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(glade_xml_get_widget(xml, "session")), &treeiter) )
	{
		return;
	}
		
	gchar *session;
	
	gtk_tree_model_get (GTK_TREE_MODEL(sessionmenu), &treeiter, 1, &session, -1);
	selected_session = session;
	
	
	g_free(session);
}

void
PubSub_Widget::SessionChange(GtkComboBox *widget, PubSub_Widget *c)
{	
	c->SesChange();
}
