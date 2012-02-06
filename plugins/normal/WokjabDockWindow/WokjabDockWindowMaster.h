/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2011 <nedo80@gmail.com>
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

#ifndef WOKJABDOCKWINDOWMASTER_H_
#define WOKJABDOCKWINDOWMASTER_H_

#include <gtkmm.h>
#include <gtkmm/socket.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>
#include <vector>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

class WokjabDockWindowMaster;

#include "wokjab-dock-window-handler.hpp"

class WokjabDockWindowMaster {
public:
	WokjabDockWindowMaster(std::string type, WokjabDockWindowHandler *handler);
	virtual ~WokjabDockWindowMaster();

	Gtk::Notebook * GetNotebook();
	void SetUrgency(bool value);
	void Raise();
	std::string getType();


	bool on_destroy(GdkEventAny *event);
	bool key_press_handler(GdkEventKey *event); 
private:
	WokjabDockWindowHandler *handler;
	
	std::string type;
	int i;
	Gtk::Window win;
	Gtk::Notebook nb;

};

#endif /* WOKJABDOCKWINDOWMASTER_H_ */
