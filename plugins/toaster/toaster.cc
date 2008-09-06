/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: Toaster
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Dec 10 04:24:23 2005
//

#include "toaster.h"
#include "ToasterWindow.h"

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk/gdkwindow.h>

#include <sstream>

#ifndef _
#define _(x) x
#endif

Toaster::Toaster(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Event Add", &Toaster::AddJIDEvent, 1000);
	EXP_SIGHOOK("Toaster Display", &Toaster::DisplayMSG, 1000);
	EXP_SIGHOOK("Toaster Remove", &Toaster::RemoveMSG, 1000);
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /toaster", &Toaster::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/toaster");
	wls->SendSignal("Config XML Trigger", &conftag);
	twid = 0;
}


Toaster::~Toaster()
{
	std::list<ToasterWindow *>::iterator iter;
	
	for( iter = twlist.begin() ; iter != twlist.end() ; iter++ )
	{
		delete (*iter);
		twlist.erase(iter);
		break;
	}
}

int
Toaster::ReadConfig(WokXMLTag *tag)
{
	
	if ( tag->GetFirstTag("config").GetFirstTag("color1").GetAttr("data").empty() )
	{
		tag->GetFirstTag("config").GetFirstTag("color1").AddAttr("type", "string");
		tag->GetFirstTag("config").GetFirstTag("color1").AddAttr("label", _("BG color 1"));
		tag->GetFirstTag("config").GetFirstTag("color1").AddAttr("data", "red");
	}
	if ( tag->GetFirstTag("config").GetFirstTag("color2").GetAttr("data").empty() )
	{
		tag->GetFirstTag("config").GetFirstTag("color2").AddAttr("type", "string");
		tag->GetFirstTag("config").GetFirstTag("color2").AddAttr("label", _("BG color 2"));
		tag->GetFirstTag("config").GetFirstTag("color2").AddAttr("data", "blue");
	}
	if ( tag->GetFirstTag("config").GetFirstTag("delay").GetAttr("data").empty() )
	{
		tag->GetFirstTag("config").GetFirstTag("delay").AddAttr("type", "string");	
		tag->GetFirstTag("config").GetFirstTag("delay").AddAttr("label", _("Delay"));
		tag->GetFirstTag("config").GetFirstTag("delay").AddAttr("data", "15");
	}
	if(  tag->GetFirstTag("config").GetFirstTag("flashes").GetAttr("data").empty() )
	{
		tag->GetFirstTag("config").GetFirstTag("flashes").AddAttr("type", "string");	
		tag->GetFirstTag("config").GetFirstTag("flashes").AddAttr("label", _("Flashes"));
		tag->GetFirstTag("config").GetFirstTag("flashes").AddAttr("data", "10");
	}
	if(  tag->GetFirstTag("config").GetFirstTag("fspeed").GetAttr("data").empty() )
	{
		tag->GetFirstTag("config").GetFirstTag("fspeed").AddAttr("type", "string");	
		tag->GetFirstTag("config").GetFirstTag("fspeed").AddAttr("label", _("Flash speed (ms)"));
		tag->GetFirstTag("config").GetFirstTag("fspeed").AddAttr("data", "200");
	}
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	return 1;	
}

int
Toaster::AddJIDEvent(WokXMLTag *tag)
{
#warning should be moved somewhere I think....

	WokXMLTag toastertag(NULL, "toaster");
	WokXMLTag &bodytag = toastertag.AddTag("body");
	bodytag.AddText(tag->GetFirstTag("item").GetFirstTag("description").GetBody());
	toastertag.AddTag(&tag->GetFirstTag("item").GetFirstTag("commands"));
	
	wls->SendSignal("Toaster Display", &toastertag);

	return 1;
}

bool
Toaster::GetXWorkArea(GdkRectangle *rect)
{
	Display *x_display;
	GdkDisplay *g_display;
	GdkScreen *g_screen;
	Screen *x_screen;
	Window x_root;
	Atom xa_desktops, xa_current, xa_workarea, xa_type;
	gulong *workareas, len, fill;
	guchar *data;
	gint format;
	guint32 desktops = 0, current = 0;

	/* get the gdk display */
	g_display = gdk_display_get_default();
	if(!g_display)
		return FALSE;

	/* get the x display from the gdk display */
	x_display = gdk_x11_display_get_xdisplay(g_display);
	if(!x_display)
		return FALSE;


	/* get the screen according to the prefs */
	g_screen = gdk_display_get_screen(g_display, 0);
	if(!g_screen)
		return FALSE;

	/* get the x screen from the gdk screen */
	x_screen = gdk_x11_screen_get_xscreen(g_screen);
	if(!x_screen)
		return FALSE;

	/* get the root window from the screen */
	x_root = XRootWindowOfScreen(x_screen);

	/* find the _NET_NUMBER_OF_DESKTOPS atom */
	xa_desktops = XInternAtom(x_display, "_NET_NUMBER_OF_DESKTOPS", True);
	if(xa_desktops == None)
		return FALSE;
		
	

	/* get the number of desktops */
	if(XGetWindowProperty(x_display, x_root, xa_desktops, 0, 1, False,
						  XA_CARDINAL, &xa_type, &format, &len, &fill,
						  &data) != Success)
	{
		return FALSE;
	}
	
	if(!data)
		return FALSE;

	desktops = *(guint32 *)data;
	XFree(data);

	/* find the _NET_CURRENT_DESKTOP atom */
	xa_current = XInternAtom(x_display, "_NET_CURRENT_DESKTOP", True);
	if(xa_current == None)
		return FALSE;

	/* get the current desktop */
	if(XGetWindowProperty(x_display, x_root, xa_current, 0, 1, False,
						  XA_CARDINAL, &xa_type, &format, &len, &fill,
						  &data) != Success)
	{
		return FALSE;
	}

	if(!data)
		return FALSE;

	current = *(guint32 *)data;
	XFree(data);

	/* find the _NET_WORKAREA atom */
	xa_workarea = XInternAtom(x_display, "_NET_WORKAREA", True);
	if(xa_workarea == None)
		return FALSE;

	if(XGetWindowProperty(x_display, x_root, xa_workarea, 0, (glong)(4 * 32),
						  False, AnyPropertyType, &xa_type, &format, &len,
						  &fill, &data) != Success)
	{
		return FALSE;
	}

	/* make sure the type and format are good */
	if(xa_type == None || format == 0)
		return FALSE;

	/* make sure we don't have any leftovers */
	if(fill)
		return FALSE;

	/* make sure len divides evenly by 4 */
	if(len % 4)
		return FALSE;

	/* it's good, lets use it */
	workareas = (gulong *)(guint32 *)data;

	rect->x = (guint32)workareas[current * 4];
	rect->y = (guint32)workareas[current * 4 + 1];
	rect->width = (guint32)workareas[current * 4 + 2];
	rect->height = (guint32)workareas[current * 4 + 3];

	/* clean up our memory */
	XFree(data);
}

int
Toaster::RemoveMSG(WokXMLTag *tag)
{
	std::list<ToasterWindow *>::iterator iter;
	
	
	for( iter = twlist.begin() ; iter != twlist.end() ; iter++ )
	{
		if ( tag->GetAttr("id") == (*iter)->GetID() )
		{
			delete (*iter);
			twlist.erase(iter);
			break;
		}
	}
	
	int Height = work_height;
	for ( iter = twlist.begin() ; iter != twlist.end() ; iter++)
	{
		(*iter)->MoveTo(work_width, Height);
		Height -= (*iter)->GetHeight();	
	}
	
	return 1;
}

int
Toaster::DisplayMSG(WokXMLTag *tag)
{
	GdkDisplay *display;
	GdkScreen *screen;
	GdkRectangle m_geo, w_geo;
	GdkRectangle rect_workspace;

	display = gdk_display_get_default();
	screen = gdk_display_get_screen(display, 0);
	gdk_screen_get_monitor_geometry(screen, 0, &m_geo);


	if( GetXWorkArea(&w_geo) )
	{
		gdk_rectangle_intersect(&w_geo, &m_geo, &rect_workspace);
	} 
	else 
	{
		rect_workspace.x = m_geo.x;
		rect_workspace.y = m_geo.y;
		rect_workspace.width = m_geo.width;
		rect_workspace.height = m_geo.height;
	}
	work_height = rect_workspace.y + rect_workspace.height;
	work_width = rect_workspace.x + rect_workspace.width;

	ToasterWindow *tw;
	std::list<ToasterWindow *>::iterator iter;
	int Height=rect_workspace.y+rect_workspace.height;
	
	for( iter = twlist.begin() ; iter != twlist.end() ; iter++ )
	{
		Height -= (*iter)->GetHeight(); 
	}
	
	std::stringstream twid_str;
	twid_str << twid++;
	tag->AddAttr("id", twid_str.str());
	
	tw = new ToasterWindow(wls, config, tag, rect_workspace.x+rect_workspace.width, Height);
	twlist.push_back(tw);
	
	return true;
}
