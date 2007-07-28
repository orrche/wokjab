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

#ifndef _G_M_WIDGET_HPP_
#define _G_M_WIDGET_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <glade/glade.h>

using namespace Woklib;
class GM_Widget;

#include "group-manager.hpp"

class GM_Widget: public WLSignalInstance 
{
public:
	GM_Widget(WLSignal *wls, GroupManager *dm);
	~GM_Widget();
	
	static void SessionChange(GtkComboBox *widget, GM_Widget *c);
	static void GroupChange(GtkTreeView *tree_view, GM_Widget *c);
protected:
	GroupManager *gm;
	GladeXML *xml;
	GtkListStore *sessionmenu;
	GtkListStore *grouplist;
	GtkListStore *inlist;
	GtkListStore *rosterlist;
	
	std::string selected_session;
	
};

#endif // _G_M_WIDGET_HPP_
