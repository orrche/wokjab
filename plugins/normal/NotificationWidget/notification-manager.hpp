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

#ifndef _NOTIFICATION_MANAGER_HPP_
#define _NOTIFICATION_MANAGER_HPP_

#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "notification-widget.hpp"

using namespace Woklib;

class NotificationManager: public WoklibPlugin 
{
public:
	NotificationManager(WLSignal *wls);
	
	int Add(WokXMLTag *tag);
	int Remove(WokXMLTag *tag);
	int GUIWindowInit(WokXMLTag *tag);
	int AddJIDEvent(WokXMLTag *tag);
	int RemoveJIDEvent(WokXMLTag *tag);
		
			
	static void CloseButton(GtkWidget *widget, NotificationManager *c);
	static void LeftButton(GtkWidget *widget, NotificationManager *c);
	static void RightButton(GtkWidget *widget, NotificationManager *c);
		
	void Update();
protected:

private:
	GladeXML *gxml;

	GtkWidget *mainwindowplug;
	WokXMLTag *inittag;
	std::list <NotificationWidget *> items;
	std::list <NotificationWidget *>::iterator pos;
		
		
		
	int id;
};

#endif // _NOTIFICATION_MANAGER_HPP_
