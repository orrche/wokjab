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

#ifndef _ROSTER_EVENT_HPP_
#define _ROSTER_EVENT_HPP_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class RosterEvent : public WoklibPlugin 
{
public:
	RosterEvent(WLSignal *wls);
	~RosterEvent();

	virtual std::string GetInfo() {return "Roster Event";};
	virtual std::string GetVersion() {return VERSION;};

	int GUIWindowInit(WokXMLTag *tag);
	int Close(WokXMLTag *tag);
	int Presence(WokXMLTag *tag);
	int ReadConfig(WokXMLTag *tag);

	std::string GetTimeStamp(time_t t);
	void SaveConfig();
	void CreateWid();
protected:
	std::list <std::string> last_events;
	GladeXML *xml;
	GtkWidget *mainwindowplug;
	int timeoutid;
	int contextid;
	WokXMLTag *inittag;
	WokXMLTag *config;
private:

};

#endif // _ROSTER_EVENT_HPP_
