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

#ifndef _NOTIFICATION_WIDGET_HPP_
#define _NOTIFICATION_WIDGET_HPP_

#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class NotificationWidget: public WLSignalInstance 
{
public:
	NotificationWidget(WLSignal *wls, WokXMLTag *tag);
		
	static gboolean CommandExec(GtkWidget *button, GdkEventButton *event, NotificationWidget *c);
			
	GtkWidget *GetWidget();
	std::string GetId();
protected:

private:
	WokXMLTag *origxml;
	GtkWidget *box;
};

#endif // _NOTIFICATION_WIDGET_HPP_
