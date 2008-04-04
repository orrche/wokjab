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

#ifndef _HISTORY_HPP_
#define _HISTORY_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <glib.h>
#include <sqlite3.h>

using namespace Woklib;

class History: public WoklibPlugin 
{
public:
	History(WLSignal *wls);
	~History();
	
	int Outgoing(WokXMLTag *tag);
	int Incomming(WokXMLTag *tag);
	int GetLast(WokXMLTag *tag);
	void PushToSQL(std::string command);
	
protected:
	sqlite3 *db;
	std::string lingering_command;
private:
	int pid;
};

#endif // _HISTORY_HPP_
