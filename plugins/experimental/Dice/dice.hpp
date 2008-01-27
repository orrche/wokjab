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

#ifndef _DICE_HPP_
#define _DICE_HPP_


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class Dice: public WoklibPlugin 
{
public:
	Dice(WLSignal *wls);
	~Dice();
	
	int MainMenu(WokXMLTag *tag);
	int DiceWid(WokXMLTag *tag);
	int ReadConfig(WokXMLTag *tag);
	
	void CreateWid();
	
	static gboolean DeleteEvent( GtkWidget *widget, GdkEvent *event, Dice *c);
	static void NewSessionMenu(GtkMenuItem *menuitem, Dice *c);
	static void NewSession(GtkMenuItem *menuitem, Dice *c);
	static void MenuItemDestroy(GtkObject *object, Dice *c);
	static void DataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time, Dice *c);
	static void DragGet(GtkWidget *wgt, GdkDragContext *contect, GtkSelectionData *selection, guint info, guint time, Dice *c);
protected:
	GladeXML *gxml;
	GtkWidget *menu;
	WokXMLTag *config;

	std::map <std::string, std::map <std::string, GladeXML* > > session_gxml;
	GtkListStore *collection_store;
private:

};

#endif // _DICE_HPP_
