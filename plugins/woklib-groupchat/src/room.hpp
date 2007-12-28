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

#ifndef _ROOM_HPP_
#define _ROOM_HPP_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokLib.h>
#include <Woklib/WLSignal.h>

#include "user.hpp"

using namespace Woklib;

class Room: public WLSignalInstance 
{
public:
	Room(WLSignal *wls);
	void AddUser(std::string username, std::string affiliation);
	void RemoveUser(std::string username);
	void SetAffiliation(std::string username, std::string affiliation);
protected:
	std::map<std::string, std::list <User*> > users;
private:

		
};

#endif // _ROOM_HPP_
