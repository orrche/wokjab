/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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


#ifndef _XDATAMULTILIST_KE_H_
#define _XDATAMULTILIST_KE_H_
#include <gtk/gtk.h>
#include "../include/xdatabase.h"

class xdatamultilist : public xdatabase
{
	public:
		xdatamultilist(WLSignal *wls, WLSignalData* wlsd, GtkWidget *vbox);
		 ~xdatamultilist();
	
		bool Ready();
		void GetData(WokXMLTag &tag);
	
		static void Add(GtkWidget *button, xdatamultilist *c);
		static void Del(GtkWidget *button, xdatamultilist *c);
		static void Up(GtkWidget *button, xdatamultilist *c);
		static void Down(GtkWidget *button, xdatamultilist *c);
	protected:
		GtkWidget *selectedlist;
		GtkWidget *optionlist;
		
		GtkListStore *selectedstore;
		GtkListStore *optionstore;
};


#endif // _XDATAMULTILIST_H_

