/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: WokPython
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Mar 13 01:50:33 2006
//

#ifndef _WOKPYTHON_H_
#define _WOKPYTHON_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "Python.h"
#include "pythread.h"
#include "PyScript.h"
#include <map>

/**
 * Class to handle request to load diffrent python scripts and unload and 
 * present a list of loaded scripts in short a manager for python scripts
 */
class WokPython : public WoklibPlugin
{
	public:
		WokPython(WLSignal *wls);
		 ~WokPython();
		
		int Load(WokXMLTag *tag);
		int Unload(WokXMLTag *tag);
		int GetScripts(WokXMLTag *tag);									/// Gets list of loaded scripts
		int Config(WokXMLTag *tag); 										/// Config to read for what scripts to load
	protected:
		std::map <std::string, PyScript*> script; 			/// List of stored scripts
		WokXMLTag *config;
};


#endif	//_WOKPYTHON_H_

