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
#ifndef _WOKJABXDATAWIDGET_H_
#define _WOKJABXDATAWIDGET_H_

#include <gtk/gtk.h>
#include <list>
#include "../include/xdatabase.h"
#include <Woklib/WLSignal.h>

class WokJabxdataWidget : public WLSignalInstance
{
	public:
		WokJabxdataWidget(WLSignal *wls,WokXMLTag *tag_iq);
		 ~WokJabxdataWidget();
	
		int GetData(WokXMLTag *tag);
	
		static void CancelButton (GtkWidget * widget, WokJabxdataWidget *c);
		static void OkButton (GtkWidget * widget, WokJabxdataWidget *c);
		static void Destroy(GtkWidget *widget, WokJabxdataWidget *c);
	protected:	
		std::string pre_msg;
		std::string post_msg;
		std::string starttag_sig;
		std::string bodytag_sig;
		std::string endtag_sig;
	
		std::string title;
		std::string instructions;
	
		std::string replsig;
	
		bool title_tag;
		bool instructions_tag;
		bool unknown_tag;
	
		GtkWidget *window;
		GtkWidget *main_hbox;
		GtkWidget *instructions_label;
		GtkWidget *ok_button;
		GtkWidget *cancel_button;
		int depth;
		std::list<xdatabase *> widget_list;
		xdatabase * current_widget;		
};


#endif	//_WOKJABXDATAWIDGET_H_
