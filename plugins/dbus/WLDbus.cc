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


#include <algorithm>

#include "WLDbus.h"
#include "wokjab-dbus-glue.h"

G_DEFINE_TYPE(WLDbus_obj, wokjab, G_TYPE_OBJECT);

WLDbus::WLDbus(WLSignal *wls): WoklibPlugin(wls)
{
	server = static_cast<WLDbus_obj*>(g_object_new(wokjab_get_type(), NULL));
	server->data = this;
}

WLDbus::~WLDbus()
{
	if ( ! hooklist.empty() )
	{
		std::list <WLDbushook*>::iterator iter;
		for ( iter = hooklist.begin() ; iter != hooklist.end() ; iter++)
		{
			delete *iter;
		}
	}

}

void
WLDbus::DeleteHook(WLDbushook *h)
{
	
	hooklist.erase(std::find(hooklist.begin(), hooklist.end(), h));
	delete h;
}

void
WLDbus::Hook(std::string signal, std::string path, std::string interface, std::string method, int prio)
{
	hooklist.push_back(new WLDbushook(wls,this, signal, path, interface, method, prio));
}

void
WLDbus::UnHook(std::string signal, std::string path, std::string interface, std::string method, int prio)
{
	if ( ! hooklist.empty() )
	{
		std::list <WLDbushook*>::iterator iter;
		for ( iter = hooklist.begin() ; iter != hooklist.end() ; iter++)
		{
			if ( (*iter)->is(signal,path,interface,method,prio) )
			{
				hooklist.erase(iter);
				delete *iter;
				break;
			}
		}
	}
}

void 
wokjab_class_init(WLDbus_objClass *cl) {
	// Nothing here
}

void
wokjab_init(WLDbus_obj *server) {
	GError *error = NULL;
	DBusGProxy *driver_proxy;
	guint request_ret;
	
	// Initialise the DBus connection
	server->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (server->connection == NULL) {
		g_warning("Unable to connect to dbus: %s", error->message);
		g_error_free(error);
		return;
	}
	
	dbus_g_object_type_install_info(wokjab_get_type(), &dbus_glib_wokjab_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(server->connection, "/net/sourceforge/wokjab", G_OBJECT(server));

	// Register the service name, the constants here are defined in dbus-glib-bindings.h
	driver_proxy = dbus_g_proxy_new_for_name(server->connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	if (!org_freedesktop_DBus_request_name (driver_proxy, "net.sourceforge.wokjab", 0, &request_ret, &error)) {
		g_warning("Unable to register service: %s", error->message);
		g_error_free(error);
	}
	
	g_object_unref(driver_proxy);
}

gboolean WLDbus_SendSignal(WLDbus_obj *obj, gchar *name, gchar *data, gchar **apple, GError **error)
{
	WokXMLTag dataxml(NULL, "data");
	dataxml.Add(data);
	if ( dataxml.GetTags().empty() )
			return TRUE;

	obj->data->wls->SendSignal(name, **dataxml.GetTags().begin());
	
	*apple = g_strdup((*dataxml.GetTags().begin())->GetStr().c_str());
	return TRUE;
}

gboolean WLDbus_HookSignal(WLDbus_obj *obj, gchar *signal, gchar *path, gchar *interface, gchar *method, int prio, gchar **apple, GError **error)
{
	obj->data->Hook(signal, path, interface, method, prio);
	return TRUE;
}

gboolean WLDbus_UnHookSignal(WLDbus_obj *obj, gchar *signal, gchar *path, gchar *interface, gchar *method, int prio, gchar **apple, GError **error)
{
	obj->data->UnHook(signal, path, interface, method, prio);
	return TRUE;
}


