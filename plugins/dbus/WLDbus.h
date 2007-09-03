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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace Woklib;
class WLDbus;

#include "w-l-dbushook.hpp"

#include <dbus/dbus-glib-bindings.h>




typedef struct
{
	GObject parent;
	DBusGConnection *connection;
	class WLDbus *data;
} WLDbus_obj;

typedef struct
{
	GObjectClass parent_class;
} WLDbus_objClass;

gboolean WLDbus_SendSignal(WLDbus_obj *obj, gchar *name, gchar *data, gchar **apple, GError **error);
gboolean WLDbus_HookSignal(WLDbus_obj *obj, gchar *signal, gchar *path, gchar *interface, gchar *method, int prio, gchar **apple, GError **error);
gboolean WLDbus_UnHookSignal(WLDbus_obj *obj, gchar *signal, gchar *path, gchar *interface, gchar *method, int prio, gchar **apple, GError **error);


class WLDbus : public WoklibPlugin
{
	friend gboolean WLDbus_SendSignal(WLDbus_obj *obj, gchar *name, gchar *data, gchar **apple, GError **error);
	friend gboolean WLDbus_HookSignal(WLDbus_obj *obj, gchar *signal, gchar *path, gchar *interface, gchar *method, int prio, gchar **apple, GError **error);
	friend gboolean WLDbus_UnHookSignal(WLDbus_obj *obj, gchar *signal, gchar *path, gchar *interface, gchar *method, int prio, gchar **apple, GError **error);
	
	public:
		WLDbus(WLSignal *wls);
		 ~WLDbus();
		
		void DeleteHook(WLDbushook *h);
		void Hook(std::string signal, std::string path, std::string interface, std::string method, int prio);
		void UnHook(std::string signal, std::string path, std::string interface, std::string method, int prio);
	
		virtual std::string GetInfo() {return "Dbus interface for signals";};
		virtual std::string GetVersion() {return VERSION;};
	protected:
		WLDbus_obj *server;
		std::list <WLDbushook*> hooklist;
};

#endif	//_WLD_BUS_H_
