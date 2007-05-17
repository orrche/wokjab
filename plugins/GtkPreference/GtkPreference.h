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
// Class: GtkPreference
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Jun 12 21:38:32 2005
//

#ifndef _GTKPREFERENCE_H_
#define _GTKPREFERENCE_H_

#include <Woklib/WokLib.h>
#include <gtk/gtk.h>
#include "GtkPCommon.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace Woklib;

class GtkPreference : public WoklibPlugin
{
	public:
		GtkPreference(WLSignal *wls);
		 ~GtkPreference();
	
		virtual std::string GetInfo() {return "GtkPreference";};
		virtual std::string GetVersion() {return VERSION;};
	
		void CreateWid();
		int AddTreeItem(GtkTreeIter *parant, WokXMLTag &tag, std::string path);
		int CreateConfig(GtkWidget *parant, WokXMLTag *tag);
		void LocalActivate(std::string strpath);
		void CleanWidgetList();
		void SaveConfig();
		void InClassDestroy();
		bool FindSanity(WokXMLTag &tag);
	
		int SigCreateWid(WokXMLTag *tag);
		int SigMenu(WokXMLTag *tag);
		int CreateWindow(WokXMLTag *tag);
		int  ReadConfig(WokXMLTag *tag);
	
		static gboolean Delete( GtkWidget *widget, GdkEvent *event, GtkPreference *c);
		static void SaveButton (GtkWidget * widget, GtkPreference *c);
		static gboolean Activate(GtkTreeView *tree_view, GdkEventButton *event, GtkPreference *c);
		static void Destroy(GtkWidget *widget, GtkPreference *c);
	protected:
		GtkWidget *window;
		GtkWidget *treeview;
		GtkWidget *configview;
		GtkTreeStore *treestore;
		WokXMLTag *config;
		WokXMLTag *myconfig;
		std::string conf_path;
		std::list <GtkPCommon *> widgets;
};


#endif	//_GTKPREFERENCE_H_

