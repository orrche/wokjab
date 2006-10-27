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

#ifndef _XDATABASE_H_
#define _XDATABASE_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>
#include <gtk/gtk.h>

using namespace Woklib;

class xdatabase : public WLSignalInstance
{
	public:
		xdatabase(WLSignal *wls, WLSignalData* wlsd);
		virtual ~xdatabase();
	
		virtual std::string GetData();
		virtual bool Ready();
		std::string GetVar();
		virtual int xml(WLSignalData *wlsd);
	protected:
		std::string var;
		bool required;
		WokXMLTag* tag_field;
		GtkWidget *labelwid;
};


#endif	//_XDATABASE_H_
