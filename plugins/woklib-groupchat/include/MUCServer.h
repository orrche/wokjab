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



#ifndef _MUCSERVER_H_
#define _MUCSERVER_H_

#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokLib.h>
#include <Woklib/WLSignal.h>

using namespace Woklib;

class MUCServer
{
	public:
		MUCServer();
		 ~MUCServer();
	
		int Presence(WokXMLTag *tag);
		int AddRoom(std::string room);
		int LeaveRoom(std::string room);
	protected:
		// MUCServer variables
	
		// TODO: add member variables...
	
};


#endif	//_MUCSERVER_H_
