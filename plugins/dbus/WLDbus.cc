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


#include "WLDbus.h"

gboolean WLDbus_SendSignal(WLDbus_obj *obj, gchar **apple, GError **error);
gboolean WLDbus_HookSignal(WLDbus_obj *obj, gchar **apple, GError **error);
gboolean WLDbus_UnHookSignal(WLDbus_obj *obj, gchar **apple, GError **error);

#include "wokjab-dbus-glue.h"

G_DEFINE_TYPE(WLDbus_obj, wokjab, G_TYPE_OBJECT);


WLDbus::WLDbus(WLSignal *wls): WoklibPlugin(wls)
{
	server = static_cast<WLDbus_obj*>(g_object_new(wokjab_get_type(), NULL));

}

WLDbus::~WLDbus()
{


}
#if 1
void 
wokjab_class_init(WLDbus_objClass *cl) {
	// Nothing here
}
#endif

#if 1
void
wokjab_init((WLDbus_obj *server) {
	GError *error = NULL;
	DBusGProxy *driver_proxy;
	int request_ret;
	
	// Initialise the DBus connection
	server->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (server->connection == NULL) {
		g_warning("Unable to connect to dbus: %s", error->message);
		g_error_free(error);
		return;
	}
	
	dbus_g_object_type_install_info(fruitd_get_type(), &dbus_glib_fruitd_object_info);
	
	// Register DBUS path
	dbus_g_connection_register_g_object(server->connection, "/org/cornershop/FruitMaker", G_OBJECT(server));

	// Register the service name, the constants here are defined in dbus-glib-bindings.h
	driver_proxy = dbus_g_proxy_new_for_name(server->connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	if (!org_freedesktop_DBus_request_name (driver_proxy, "org.cornershop.FruitMaker", 0, &request_ret, &error)) {
		g_warning("Unable to register service: %s", error->message);
		g_error_free(error);
	}
	
	g_object_unref(driver_proxy);
}
#endif

gboolean WLDbus_SendSignal(WLDbus_obj *obj, gchar **apple, GError **error)
{
	std::cout << "Sending signal" << std::endl;
	return TRUE;
}

gboolean WLDbus_HookSignal(WLDbus_obj *obj, gchar **apple, GError **error)
{
	std::cout << "Hoooking" << std::endl;
	return TRUE;
}

gboolean WLDbus_UnHookSignal(WLDbus_obj *obj, gchar **apple, GError **error)
{
	std::cout << "Unhooking" << std::endl;
	return TRUE;
}
