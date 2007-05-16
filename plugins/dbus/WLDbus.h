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


#ifndef _WLD_BUS_H_
#define _WLD_BUS_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>



using namespace Woklib;

// #include <dbus/dbus-glib-bindings.h>

/*
typedef struct
{
	GObject parent;
	DBusGConnection *connection;
} WLDbus_obj;

typedef struct
{
	GObjectClass parent_class;
} WLDbus_objClass;
*/

class WLDbus : public WoklibPlugin
{
	public:
		WLDbus(WLSignal *wls);
		 ~WLDbus();
	
	protected:
//		WLDbus_obj *server;
};


#endif	//_WLD_BUS_H_
