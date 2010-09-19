/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2009 <nedo80@gmail.com>
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

#ifndef _CONN_WINDOW_MANAGER_H_
#define _CONN_WINDOW_MANAGER_H_

#include <gtk/gtk.h>
#include <map>

#include "GUIPluginWindow.h"

#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;


class ConnWindowManager: public WoklibPlugin 
{
public:
	ConnWindowManager(WLSignal *wls);
	~ConnWindowManager();
protected:

	int Open(WokXMLTag *tag);
	int Close(WokXMLTag *tag);

	std::map <std::string, GUIPluginWindow*> win;

	int winid;
private:

};

#endif // _CONN_WINDOW_MANAGER_H_
