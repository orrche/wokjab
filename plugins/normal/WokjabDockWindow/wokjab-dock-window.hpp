/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WOKJAB_DOCK_WINDOW_HPP_
#define _WOKJAB_DOCK_WINDOW_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>
#include <vector>
#include <gtk/gtk.h>
#include <gdl/gdl.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace Woklib;

/**
 * WokjabDock window for docking sub windows including other wokjab dock window 
 */
class WokjabDockWindow: public WLSignalInstance
{
public:
	WokjabDockWindow(WLSignal *wls, WokXMLTag *in_inittag, GtkWidget *topdock, WokjabDockWindow *relative);
	~WokjabDockWindow();
		
	static void Destroy(GdlDockObject *gdldockobject, gboolean arg1, WokjabDockWindow *c);
	static void Dock(GdlDockObject *gdldockobject, GdlDockObject *arg1, GdlDockPlacement arg2, GValue *arg3, WokjabDockWindow *c);
	static void Unrealize(GtkWidget *widget, WokjabDockWindow *c);
	static void Realize(GtkWidget *widget, WokjabDockWindow *c);
		
	std::string GetType();
protected:
	gulong sig1h;
	gulong sig2h;
	gulong sig3h;
		
	WokXMLTag *inittag;		
	GtkWidget *win;
			
	GtkWidget *mainsock;
		
	GtkWidget *hiddenwindow;
	GtkWidget *placeholder;
private:

};

#endif // _WOKJAB_DOCK_WINDOW_HPP_
