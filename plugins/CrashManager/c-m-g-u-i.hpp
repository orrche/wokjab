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

#ifndef _C_M_G_U_I_HPP_
#define _C_M_G_U_I_HPP_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WokLib.h>
#include <glade/glade.h>
#include <gtk/gtk.h>

using namespace Woklib;

class CMGUI;

#include "crash-manager.hpp"

class CMGUI: public WLSignalInstance 
{
public:
	CMGUI(WLSignal *wls, CrashManager *parant);
	~CMGUI();
	
	int ReadConfig(WokXMLTag *tag);
	
	static void ChatButton(GtkButton *widget, CMGUI *c);
	static void SendButton(GtkButton *widget, CMGUI *c);
	static gboolean Delete( GtkWidget *widget, GdkEvent *event, CMGUI *c);
	
protected:
	CrashManager *parant;
	WokXMLTag *config;
	GladeXML *xml;
	GtkListStore *sessionmenu;
private:

};

#endif // _C_M_G_U_I_HPP_
