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

#ifndef _PUB_SUB_WIDGET_HPP_
#define _PUB_SUB_WIDGET_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace Woklib;

class PubSub_Widget;

#include "pub-sub-manager.hpp"

class PubSub_Widget: public WLSignalInstance 
{
public:
	PubSub_Widget(WLSignal *wls, WokXMLTag *data, PubSubManager *parant);
	~PubSub_Widget();
	
	int ReadConfig(WokXMLTag *tag);
	int ConfigIQResp(WokXMLTag *tag);
	int RegisterIQResp(WokXMLTag *tag);
	int ReadIQResp(WokXMLTag *tag);
	int XdataResp(WokXMLTag *tag);
		
	void SesChange();
	void SaveConfig();
	
	void ConfButton();
	void RegButton();
	void ReadBtn();
	void SaveList();
		
	static void SessionChange(GtkComboBox *widget, PubSub_Widget *c);
	static gboolean Delete( GtkWidget *widget, GdkEvent *event, PubSub_Widget *c);
	static void FindJidButton(GtkButton *button, PubSub_Widget *c);
	static void FindNodeButton(GtkButton *button, PubSub_Widget *c);
	static void ConfigButton(GtkButton *button, PubSub_Widget *c);
	static void RegisterButton(GtkButton *button, PubSub_Widget *c);
	static void ReadButton(GtkButton *button, PubSub_Widget *c);
	static void cell_edited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, PubSub_Widget *c);
	static void editing_started (GtkCellRenderer *cell, GtkCellEditable *editable, const gchar *path, PubSub_Widget *c);
	static void ApplyButton(GtkButton *button, PubSub_Widget *c);
	static void AddButton(GtkButton *button, PubSub_Widget *c);
	static void DeleteButton(GtkButton *button, PubSub_Widget *c);
	static void SubscribeButton(GtkButton *button, PubSub_Widget *c);
	static void UnsubscribeButton(GtkButton *button, PubSub_Widget *c);
	static void UnregisterButton(GtkButton *button, PubSub_Widget *c);
protected:
		
	GtkListStore *sessionmenu;
	GtkListStore *jidmenu;
	GtkListStore *nodemenu;
	GtkListStore *affiliationlist;
	GtkListStore *aff_list;
	GtkBuilder *builder;
	
	std::string selected_session;
	
	PubSubManager *parant;
	WokXMLTag *config;
private:

};

#endif // _PUB_SUB_WIDGET_HPP_
