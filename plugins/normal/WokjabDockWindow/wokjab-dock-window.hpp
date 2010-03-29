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

#include <gtkmm.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>
#include <vector>

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
	WokjabDockWindow(WLSignal *wls, WokXMLTag *in_inittag, Gtk::Notebook *parent);
	~WokjabDockWindow();

	/*
	 
	static void Destroy(GdlDockObject *gdldockobject, gboolean arg1, WokjabDockWindow *c);
	static void Dock(GdlDockObject *gdldockobject, GdlDockObject *arg1, GdlDockPlacement arg2, GValue *arg3, WokjabDockWindow *c);
	 */
	static void Unrealize(GtkWidget *widget, WokjabDockWindow *c);
	static void Realize(GtkWidget *widget, WokjabDockWindow *c);
	static void LabelUnrealize(GtkWidget *widget, WokjabDockWindow *c);
	static void LabelRealize(GtkWidget *widget, WokjabDockWindow *c);
	
	std::string GetType();
	void SetUrgencyHint(WokXMLTag *tag);
	void Activate();
		
	bool Visible();
	void Show(WokXMLTag *tag);
	void Hide(WokXMLTag *tag);
		
	static gboolean key_press_handler(GtkWidget * widget, GdkEventKey * event,
			     WokjabDockWindow *c);
protected:
	gulong sig1h;
	gulong sig2h;
	gulong sig3h;
	gulong sig2lh;
	gulong sig3lh;
	
	WokXMLTag *inittag;
	Gtk::Notebook *parent;
	GtkWidget *win;
	GtkWidget *topdock;
		
	GtkWidget *mainsock;
	GtkWidget *hiddenwindow;
	GtkWidget *placeholder;

	GtkWidget *labelsock;
	GtkWidget *labelph;
	GtkWidget *hiddenlabel;
private:

};

#endif // _WOKJAB_DOCK_WINDOW_HPP_
