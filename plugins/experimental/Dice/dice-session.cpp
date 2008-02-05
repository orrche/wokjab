/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
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

#include "dice-session.hpp"
#include <sstream>

DiceSession::DiceSession(WLSignal *wls, std::string session, std::string roomjid, GladeXML *parant_gxml) : WLSignalInstance(wls), 
session(session),
roomjid(roomjid),
parant_gxml(parant_gxml)
{
	EXP_SIGHOOK("Jabber Dice Message '" + XMLisize(session) + "' '" + XMLisize(roomjid) + "'", &DiceSession::Message, 1000);
	
	roll_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	hist_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/dice.glade", "mainbox", NULL);
		
	WokXMLTag whoami("whoami");
	wls->SendSignal("Jabber GroupChat Whoami '" + XMLisize(session) + "' '" + XMLisize(roomjid) + "'", whoami);
	mynick = whoami.GetAttr("nick");
	
	gtk_notebook_append_page(GTK_NOTEBOOK(glade_xml_get_widget (parant_gxml, "notebook")), glade_xml_get_widget (gxml, "mainbox"), gtk_label_new((session + ":" + roomjid).c_str()));
			
	GtkCellRenderer *renderer;
	
	/*
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "current_roll")), renderer, TRUE);
	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "current_roll")), renderer, set_cell_color, NULL, NULL);
	*/
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 0.5, "yalign", 0.5, NULL);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "current_roll")), renderer, TRUE);
	gtk_icon_view_set_model(GTK_ICON_VIEW(glade_xml_get_widget (gxml, "current_roll")), GTK_TREE_MODEL(roll_store));
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "current_roll")), renderer, "text", 0, NULL);	
	
	
	GtkTreeViewColumn *column;
	gtk_tree_view_set_model(GTK_TREE_VIEW(glade_xml_get_widget (gxml, "history_view")), GTK_TREE_MODEL(hist_store));
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("User",
							     renderer, "text",
							     0, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "history_view")), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Result",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "history_view")), column);
	
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "roll_button")), "clicked" , G_CALLBACK(DiceSession::StartRoll), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "clear_history_button")), "clicked" , G_CALLBACK(DiceSession::ClearHistory), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "clear_button")), "clicked" , G_CALLBACK(DiceSession::ClearSelection), this);
	
    enum
    {
      TARGET_STRING,
      TARGET_URL
    };
	static GtkTargetEntry target_entry[] =
    {
      { "STRING",        0, TARGET_STRING },
      { "text/plain",    0, TARGET_STRING },
    };
	gtk_drag_dest_set(glade_xml_get_widget (gxml, "current_roll"), GTK_DEST_DEFAULT_ALL, target_entry, 2, (GdkDragAction) (GDK_ACTION_COPY|GDK_ACTION_MOVE|GDK_ACTION_LINK));
	//gtk_icon_view_enable_model_drag_dest(GTK_ICON_VIEW(glade_xml_get_widget (c->session_gxml[session][roomjid], "current_roll")), target_entry, 3, (GdkDragAction) (GDK_ACTION_COPY));
	g_signal_connect(glade_xml_get_widget (gxml, "current_roll"), "drag_data_received", G_CALLBACK(DiceSession::DataReceived), this);
	
	
}

DiceSession::~DiceSession()
{
	
	
	
}

void
DiceSession::AddToSession(WokXMLTag &die)
{
	GtkTreeIter iter;
	gtk_list_store_append (roll_store, &iter);
	
	gtk_list_store_set (roll_store, &iter, 0, (die.GetFirstTag("name").GetBody() + " [" + die.GetFirstTag("type", "common notation").GetBody() + "]").c_str() , 2 , new WokXMLTag (die), -1);
}

void
DiceSession::Roll()
{
	WokXMLTag RandID("rand");
	wls->SendSignal("Jabber RandomNumber RegisterSesseion", RandID);
	
	WokXMLTag dice_mesg("message");
	dice_mesg.AddAttr("session", session);
	WokXMLTag &message = dice_mesg.AddTag("message");
	message.AddAttr("to", roomjid);
	message.AddAttr("type", "groupchat");
	WokXMLTag &dice_x = message.AddTag("x", "http://wokjab.sf.net/dice");
	WokXMLTag &rand = dice_x.AddTag("rand", "RandomNumber");
	WokXMLTag &rand_id = rand.AddTag("id");
	rand_id.AddAttr("id", RandID.GetAttr("id"));
	rand_id.AddAttr("owner", roomjid + "/" + mynick);
	WokXMLTag &dice_tag = dice_x.AddTag("dice");
	
	GtkTreeIter iter;
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(roll_store), &iter) )
	{
		do
		{
			WokXMLTag *tag;
			gtk_tree_model_get(GTK_TREE_MODEL(roll_store), &iter, 2, &tag, -1);
			dice_tag.AddTag(tag);
		}while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(roll_store), &iter) == TRUE);
	}
	else
	{
		// Come on...
		return;
	}
	

		
	wls->SendSignal("Jabber XML Send", dice_mesg);
	
	WokXMLTag p2prand("rand");
	p2prand.AddAttr("session", session);
	p2prand.AddAttr("roomjid", roomjid);
	p2prand.AddAttr("id", RandID.GetAttr("id"));
	
	wls->SendSignal("Jabber RandomNumber SessionCreate", p2prand);
}

int
DiceSession::Message(WokXMLTag *tag)
{
	WokXMLTag &idtag = tag->GetFirstTag("message").GetFirstTag("x", "http://wokjab.sf.net/dice").GetFirstTag("rand", "RandomNumber").GetFirstTag("id");
	
	rand_num[idtag.GetAttr("owner")] = idtag.GetAttr("id");
	dice_data[idtag.GetAttr("owner")] = new WokXMLTag(tag->GetFirstTag("message").GetFirstTag("x", "http://wokjab.sf.net/dice").GetFirstTag("dice"));
	
	EXP_SIGHOOK("Jabber RandomNumber Generated '" + XMLisize(session) + "' '" + XMLisize(idtag.GetAttr("owner")) + "' '" + XMLisize(idtag.GetAttr("id")) + "'", &DiceSession::RandNR, 1000);
	
	return 1;
}

int 
DiceSession::RandNR(WokXMLTag *tag)
{	
	unsigned long long int num = strtoul(("0x" + tag->GetAttr("num")).c_str(), NULL, 16);
	unsigned long long int bits = 0xFFFFFFFFFFFFFFFFULL;
		
	std::stringstream result_str;
	
	std::list <WokXMLTag *>::iterator dieiter;
	for( dieiter = dice_data[tag->GetAttr("owner")]->GetTagList("die").begin(); dieiter != dice_data[tag->GetAttr("owner")]->GetTagList("die").end() ; dieiter++)
	{
		unsigned long long int faces = atoi((*dieiter)->GetFirstTag("type", "common notation").GetBody().substr(1).c_str());
		
		if ( faces > bits )
		{
			woklib_message(wls, "we have run out of random data");
			break;
		}
		
		unsigned long long int result = num%faces + 1;
		num /= faces;
		bits /= faces;
		
		if( result_str.str().size() )
			result_str << "\n";
		result_str << (*dieiter)->GetFirstTag("name").GetBody() << " [" << (*dieiter)->GetFirstTag("type","common notation").GetBody() << "]:\t" << result;
	}
	
	GtkTreeIter iter;
	gtk_list_store_prepend (hist_store, &iter);
	
	std::string nick = tag->GetAttr("owner").substr(tag->GetAttr("owner").find("/")+1);
	gtk_list_store_set (hist_store, &iter, 0, nick.c_str() , 1 , result_str.str().c_str(), -1);
	
	return 1;
}

void
DiceSession::StartRoll(GtkButton *button, DiceSession *c)
{
	c->Roll();
}

void
DiceSession::ClearSelection(GtkButton *button, DiceSession *c)
{
	gtk_list_store_clear(GTK_LIST_STORE(c->roll_store));
}


void
DiceSession::ClearHistory(GtkButton *button, DiceSession *c)
{
	gtk_list_store_clear(GTK_LIST_STORE(c->hist_store));
}

void
DiceSession::DataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time, DiceSession *c)
{	
	WokXMLTag data("data");
	data.Add((const char*)gtk_selection_data_get_text (seldata));
	
	std::list <WokXMLTag *>::iterator iter;
	for( iter = data.GetFirstTag("dice").GetTagList("die").begin() ; iter != data.GetFirstTag("dice").GetTagList("die").end(); iter++)
		c->AddToSession(**iter);
}
