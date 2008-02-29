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

#ifndef _EVENT_MANAGER_HPP_
#define _EVENT_MANAGER_HPP_


#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>
#include <map>

using namespace Woklib;

class EventManager: public WoklibPlugin 
{
public:
	EventManager(WLSignal *wls);
	~EventManager(); 
	
	int Remove(WokXMLTag *tag);
	int Add(WokXMLTag *tag);
	int Timeout(WokXMLTag *tag);
	
protected:
	int id;
private:

};

#endif // _EVENT_MANAGER_HPP_
