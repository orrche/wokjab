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

#ifndef _WOKJAB_DOCK_WINDOW_HANDLER_HPP_
#define _WOKJAB_DOCK_WINDOW_HANDLER_HPP_


#include <gtkmm.h>
#include "wokjab-dock-window.hpp"

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>


#include <iostream>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace Woklib;

/**
 * Class to handle the creation and deletion of wokjab dock windows
 */
class WokjabDockWindowHandler: public WoklibPlugin 
{
public:
	WokjabDockWindowHandler(WLSignal *wls);
		
	int Add(WokXMLTag *tag);
	int Destroy(WokXMLTag *tag);
	int Activate(WokXMLTag *tag);
	int SetUrgency(WokXMLTag *tag);
		
	int Hide(WokXMLTag *tag);
	int Show(WokXMLTag *tag);
protected:
	std::map <std::string, Gtk::Notebook *> masterlist;
	std::map <std::string, WokjabDockWindow*> windows;
		
	std::vector <int> Widgets;
	std::vector <int> HiddenWidgets;
	std::map <int, GtkWidget *> sockets;
	std::map <int, GtkWidget *> labelsockets;
	std::map <int, std::string> identifier;
private:

};

#endif // _WOKJAB_DOCK_WINDOW_HANDLER_HPP_
