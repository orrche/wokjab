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
// Class: GtkPText
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Aug 14 19:34:05 2005
//

#include "GtkPText.h"


GtkPText::GtkPText(WokXMLTag *tag) : GtkPCommon(tag)
{
	std::string name;
	if( tag->GetAttr("label").size() )
		name = tag->GetAttr("label");
	else
		name = tag->GetName();
		
	GtkWidget *label = gtk_label_new(name.c_str());
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_IN);
	
	textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview),
				     GTK_WRAP_WORD);
	
	vbox = gtk_vbox_new(FALSE, 2);
	
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	std::string text = tag->GetBody();
	gtk_text_buffer_set_text (buffer, text.c_str(), text.size());
	
	gtk_container_add(GTK_CONTAINER(scroll), textview);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, FALSE, FALSE, 2);
	
	gtk_widget_show_all(vbox);
}


GtkPText::~GtkPText()
{
	
}

GtkWidget *
GtkPText::GetWidget()
{
	return vbox;
}	

void
GtkPText::Save()
{
	std::string str;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	
	GtkTextIter start_iter, end_iter;
		
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
		
	str = gtk_text_buffer_get_text(buffer,&start_iter, &end_iter, false);
		
	conf->RemoveBody();
	conf->AddText(str);
}

