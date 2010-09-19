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

#ifndef _P_E_P_USER_HPP_
#define _P_E_P_USER_HPP_

#include <glade/glade.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

class PEPUser;

#include "p-e-p-widget.hpp"

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class PEPUser: public WLSignalInstance 
{
public:
	PEPUser(WLSignal *wls, PEP_Widget *pw, std::string session, std::string jid);
	~PEPUser();
	
	int UserQueryResultInfo(WokXMLTag *tag);
	int UserQueryResultItem(WokXMLTag *tag);
	int UserQuerySubscription(WokXMLTag *tag);
	
	void Show();
	void Hide();
	void Update();
	
	bool SetNode(std::string node);
protected:

private:
	GtkTreeIter tIter;
	PEP_Widget *pw;
	std::string session,jid, currentnode;
	bool visible;
	
	std::map <std::string, int> nodelist;
		
};

#endif // _P_E_P_USER_HPP_
