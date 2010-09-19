/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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



#ifndef _GUIPRESENTREQWIDGET_H_
#define _GUIPRESENTREQWIDGET_H_


#include <gtk/gtk.h>
#include <list>

#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class GUIPresentReqWidget : public WoklibPlugin
{
	public:
		GUIPresentReqWidget(WLSignal * wls);
		 ~GUIPresentReqWidget();
	
		int Presence (WokXMLTag * msgtag);
		int vcard(WokXMLTag *tag);
		
		void create_widget ();
		void update_list();
		void AddJID(std::string jid, std::string nick, std::string session);
	
		static void Close_Button(GtkWidget * widget, GUIPresentReqWidget *req_widget);
		static void Add_Button(GtkWidget * widget, GUIPresentReqWidget *req_widget);
		static void Destroy(GtkWidget * widget, GUIPresentReqWidget *req_widget);
		static void AddAll_Button(GtkWidget *widget, GUIPresentReqWidget *req_widget);
	
	protected:
		bool widget_visible;
	
		std::map <std::string, WokXMLTag *> items;
	
		GtkWidget *window1;
		GtkWidget *jid_treeview;
	
		GtkListStore *model;

};


#endif	//_GUIPRESENTREQWIDGET_H_
