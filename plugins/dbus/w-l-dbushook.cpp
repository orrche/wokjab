/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "w-l-dbushook.hpp"


WLDbushook::WLDbushook(WLSignal *wls, WLDbus *parant, std::string signal, std::string path, std::string interface, std::string method, int prio) : WLSignalInstance(wls),
parant(parant),
signal(signal),
path(path),
interface(interface),
method(method),
prio(prio)
{
	EXP_SIGHOOK(signal, &WLDbushook::exec, prio);
}

WLDbushook::~WLDbushook()
{
	
}

bool 
WLDbushook::is(std::string signal, std::string path, std::string interface, std::string method, int prio)
{
	return ( signal == this->signal && path == this->path && interface == this->interface && method == this->method && prio == this->prio );
	
}

int
WLDbushook::exec(WokXMLTag *tag)
{
	DBusGConnection *connection;
	GError *error = NULL;
	DBusGProxy *proxy;
	int return_value = 1;
	
	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	
	if (connection == NULL)
	{
		g_printerr ("Failed to open connection to bus: %s\n",
		error->message);
		g_error_free (error);
		return 1;
    }
	
	proxy = dbus_g_proxy_new_for_name_owner (connection,
							interface.c_str(),
							path.c_str(),
							interface.c_str(), &error);
	if ( !proxy ) 
	{
		woklib_debug(wls, error->message);
		g_error_free (error);
		return 1;
	}
	/* Call ListNames method, wait for reply */
	error = NULL;
	gchar *return_xml = NULL;
	
	if (dbus_g_proxy_call (proxy, method.c_str(), &error, 
							G_TYPE_STRING, tag->GetStr().c_str(), G_TYPE_INVALID,  
							G_TYPE_INT, &return_value, G_TYPE_STRING, &return_xml, G_TYPE_INVALID) == FALSE)
	{
		/* Just do demonstrate remote exceptions versus regular GError */
		if ( error )
		{
			if (error->domain == DBUS_GERROR && error->code == DBUS_GERROR_REMOTE_EXCEPTION)
				g_printerr ("Caught remote method exception %s: %s", dbus_g_error_get_name (error), error->message);
			else
				g_printerr ("Error: %s\n", error->message);
			g_error_free (error);
		}
		parant->DeleteHook(this);
		return 1;
	}
	else
	{
		if ( return_xml && return_xml[0])
		{
			WokXMLTag dataxml(NULL, "data");
			dataxml.Add(return_xml);
			*tag = **dataxml.GetTags().begin();
		}
	}
	
	return return_value;	
}
