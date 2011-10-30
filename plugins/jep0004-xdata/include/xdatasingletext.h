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
#ifndef _XDATASINGLETEXT_H_
#define _XDATASINGLETEXT_H_

#include <gtk/gtk.h>
#include <string>
#include "../include/xdatabase.h"
#include <Woklib/WLSignal.h>

class xdatasingletext : public xdatabase
{
	public:
		xdatasingletext(WLSignal *wls, WLSignalData *wlsd, GtkWidget *vbox);
		 ~xdatasingletext();
	
		bool Ready();
		void GetData(WokXMLTag &tag);
	protected:
		GtkWidget *entry;
};


#endif	//_XDATASINGLETEXT_H_
