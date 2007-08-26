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

#ifndef _CRASH_MANAGER_HPP_
#define _CRASH_MANAGER_HPP_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WokLib.h>
#include <fstream>

using namespace Woklib;

class CrashManager: public WoklibPlugin 
{
public:
	CrashManager(WLSignal *wls);
	~CrashManager();
	
	int Sig(WokXMLTag *tag);
	int Exit(WokXMLTag *tag);
protected:
	std::string filename;
	std::ofstream file;
	bool exiting_cleanly;
private:

};

#endif // _CRASH_MANAGER_HPP_
