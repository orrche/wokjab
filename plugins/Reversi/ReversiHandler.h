/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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
// Class: ReversiHandler
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Aug 27 23:18:39 2005
//

#ifndef _REVERSIHANDLER_H_
#define _REVERSIHANDLER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <map>
#include "Game.h"

using namespace Woklib;

class ReversiHandler : public WoklibPlugin
{
	public:
		ReversiHandler(WLSignal *wls);
		 ~ReversiHandler();
	
		virtual std::string GetInfo() {return "Reversi";};
		virtual std::string GetVersion() {return "0.0.1";};
			
		int IQResp(WokXMLTag *tag);
		int Menu(WokXMLTag *tag);
		int StartSession(WokXMLTag *tag);
		int Request(WokXMLTag *tag);
		int Move(WokXMLTag *tag);
		int RemoveSession(WokXMLTag *tag);
	protected:
		
		std::map <std::string , std::map<std::string, Game *> > Session;
};


#endif	//_REVERSIHANDLER_H_

