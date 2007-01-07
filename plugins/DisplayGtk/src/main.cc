/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with main.c; if not, write to:
//            The Free Software Foundation, Inc.,
//            59 Temple Place - Suite 330,
//            Boston,  MA  02111-1307, USA.

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "DisplayGtk.h"

extern "C"
#ifdef __WIN32
__declspec(dllexport)
#endif
WoklibPlugin *maker(WLSignal *wls) {
	return reinterpret_cast <WoklibPlugin *> (new DisplayGtk(wls));
}

extern "C"
#ifdef __WIN32
__declspec(dllexport)
#endif
void destroyer(WoklibPlugin *plugin) {
        delete plugin;
}


