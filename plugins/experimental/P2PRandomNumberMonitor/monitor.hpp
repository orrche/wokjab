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

#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class Monitor: public WoklibPlugin 
{
public:
	Monitor(WLSignal *wls);
	~Monitor();
	
	int MainMenu(WokXMLTag *tag);
	int Wid(WokXMLTag *tag);
 	int Menu(WokXMLTag *tag);
	int CancelMenu(WokXMLTag *tag);
	
	int Cancel(WokXMLTag *tag);
	int Create(WokXMLTag *tag);
	int Remove(WokXMLTag *tag);
	int Generated(WokXMLTag *tag);
	int Progress(WokXMLTag *tag);
	
	void CreateWid();
	bool FindIter(const std::string &if_session, const std::string &if_id, const std::string &if_owner, const std::string &if_roomjid, GtkTreeIter &iter);
	
	static gboolean DeleteEvent( GtkWidget *widget, GdkEvent *event, Monitor *c);
	static gboolean PopupMenu(GtkTreeView *tree_view, GdkEventButton *event, Monitor *c);
protected:
	GladeXML *gxml;
	GtkListStore *sess_store;
	
	std::string menu_roomjid, menu_owner, menu_id, menu_session;
private:

};

#endif // _MONITOR_HPP_
