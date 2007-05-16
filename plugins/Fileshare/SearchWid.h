/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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
// Class: SearchWid
// Created by: Kent Gustavsson <nedo80@gmail.com>
//

#ifndef _SEARCH_WID_H_
#define _SEARCH_WID_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class SearchWid : public WLSignalInstance
{
	public:
		SearchWid(WLSignal *wls);
		~SearchWid();
		
		
		int IncommingResult(WokXMLTag *tag);
		int Menu(WokXMLTag *tag);
		
		void Hook(std::string thread);
		
		static void Search(GtkButton *button, SearchWid *c);
		static gboolean popup_menu(GtkTreeView *tree_view, GdkEventButton *event, SearchWid *c);
	protected:
		GladeXML *xml;
		GtkListStore *search_store;
		std::string menu_jid;
		std::string menu_session;
		std::string menu_file_id;
};


#endif // _SEARCH_WID_H
