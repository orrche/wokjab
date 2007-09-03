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
// Class: GtkMenu
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug  9 14:20:55 2005
//

#ifndef _GTKMENU_H_
#define _GTKMENU_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace Woklib;
/**
 * Takes care of generating menus in a nice fasion send it signals to call for 
 * items, time of button press and location and it will create a menu without 
 * fuss
 */
class GtkJabberMenu : public WoklibPlugin
{
	public:
		GtkJabberMenu(WLSignal *wls);
		 ~GtkJabberMenu();
	
		int Create(WokXMLTag *tag);
	
		static gboolean MenuActivate (GtkMenuItem *menuitem,GtkJabberMenu *c);
		void AddItem(GtkWidget *menu, WokXMLTag *tag, WokXMLTag *data);
		void CleanDataStore(); 																	/// Responsibule for the cleanup 
	
	
		virtual std::string GetInfo() {return "Menu Widget";};
		virtual std::string GetVersion() {return VERSION;};
	protected:
		std::map <GtkWidget *, std::string> MenuSignals; 														/// To store individual signals for each menu item
		std::map <GtkWidget *, WokXMLTag *> MenuData; 															/// To store special data to send for each menu item
		std::list <WokXMLTag *> DataStore; 																					/// Something to do with individual data stored in hiracracy looks like a mess of god like proportions
};


#endif	//_GTKMENU_H_

