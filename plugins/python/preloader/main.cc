/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <dlfcn.h>

using namespace Woklib;

void *__WLP_PlugInLib;
WoklibPlugin *__WLP_plugin;


extern "C" WoklibPlugin *maker(WLSignal *wls) {
	WoklibPlugin *plugin;
	WoklibPlugin *(*mkr) (WLSignal *wls);
	std::string filename = "/usr/local/lib/woklib-plugins/secondary/libwokpython.so";
	
	__WLP_PlugInLib = dlopen (filename.c_str (), RTLD_GLOBAL);
	if (!__WLP_PlugInLib)
	{
		woklib_error(wls, "Cannot load library: " + std::string(dlerror()));
		return 0;
	}
	
	mkr = (WoklibPlugin *(*)(WLSignal *wls)) dlsym (__WLP_PlugInLib, "maker");

	return __WLP_plugin = mkr (wls);
	/*
	pluginhandle[filename] = PlugInLib;
	plugins[filename] = plugin;
	
	return 1;
	
	
	
	
	return reinterpret_cast <WoklibPlugin *> (new WokPython(wls));
*/
}

extern "C" void destroyer(WoklibPlugin *plugin) {

	void(*mkr) (WoklibPlugin*);
	
	mkr = (void(*)(WoklibPlugin *)) dlsym (__WLP_PlugInLib, "destroyer");
		
	mkr(__WLP_plugin);
}


