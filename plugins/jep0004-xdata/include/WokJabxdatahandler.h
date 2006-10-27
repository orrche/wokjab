/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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
#ifndef _WOKJABXDATAHANDLER_H_
#define _WOKJABXDATAHANDLER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class WokJabxdatahandler : public WoklibPlugin
{
	public:
		WokJabxdatahandler(WLSignal *wls);
		 ~WokJabxdatahandler();
	
		virtual std::string GetInfo() {return "Jabber:x:data";};
		virtual std::string GetVersion() {return "0.0.1";};
	
		void  init(WokXMLTag *);
	
	protected:
		int id;
};


#endif	//_WOKJABXDATAHANDLER_H_
