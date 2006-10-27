/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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

#ifndef __JEP65_H
#define __JEP65_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>
#include <gtk/gtk.h>
#include "map"

#include "jep65send.h"

using namespace Woklib;

class jep65 : public WoklibPlugin
{
	public:
		jep65(WLSignal *wls);
		~jep65();

		int Session(WokXMLTag *xml);
		int Send(WokXMLTag *xml);
		int ReadData(WokXMLTag *xml);
		int DiscoInfo(WokXMLTag *xml);
	private:
		std::string sport;
		int serversock;
		std::map <std::string , jep65send*> sendingfiles;
		std::map <int, jep65send* > socketfiles;
	
};

#endif // __JEP65_H
