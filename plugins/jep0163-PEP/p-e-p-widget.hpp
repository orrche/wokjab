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

#ifndef _P_E_P_WIDGET_HPP_
#define _P_E_P_WIDGET_HPP_


#include <glade/glade.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif



class PEP_Widget;

#include "p-e-p-manager.hpp"
#include "p-e-p-user.hpp"

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class PEP_Widget: public WLSignalInstance 
{
public:
	PEP_Widget(WLSignal *wls, PEPManager *pm);
	~PEP_Widget();
	
	
	static gboolean Delete( GtkWidget *widget, GdkEvent *event, PEP_Widget *c);
	static void SessionChange(GtkComboBox *widget, PEP_Widget *c);
	static void TypeChange(GtkTreeView *tree_view, PEP_Widget *c);

	int ReadConfig(WokXMLTag *tag);
		
	void SaveConfig();
	void SesChange();
	void AddType(std::string type);
	
	GtkListStore *pep_type_list;
	GtkListStore *jid_list;
	
protected:
	GladeXML *xml;
	
	GtkListStore *sessionmenu;
	
	//std::map <std::string, std::list<std::string> > userlist;
	
	std::list <PEPUser *> userlist;
	std::list <std::string> typelist;
	
	std::string selected_session;
	PEPManager *pm;
	WokXMLTag *config;
};

#endif // _P_E_P_WIDGET_HPP_
