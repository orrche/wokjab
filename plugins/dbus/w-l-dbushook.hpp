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

#ifndef _W_L_DBUSHOOK_HPP_
#define _W_L_DBUSHOOK_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class WLDbushook;

#include "WLDbus.h"

class WLDbushook: public WLSignalInstance 
{
public:
	WLDbushook(WLSignal *wls, WLDbus *parant, std::string signal, std::string path, std::string interface, std::string method, int prio);
	~WLDbushook();
	
	bool is(std::string signal, std::string path, std::string interface, std::string method, int prio);
	int exec(WokXMLTag *tag);
protected:
	WLDbus *parant;
	std::string signal, path, method,interface;
	int prio;
private:

};

#endif // _W_L_DBUSHOOK_HPP_
