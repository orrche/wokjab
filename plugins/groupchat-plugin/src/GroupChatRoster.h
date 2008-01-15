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
// Class: GroupChatRoster
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Wed Sep  6 18:30:45 2006
//

#ifndef _GROUPCHATROSTER_H_
#define _GROUPCHATROSTER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class GroupChatRoster : public WLSignalInstance
{
	public:
		GroupChatRoster(WLSignal *wls, std::string room, std::string mainwid, std::string mynick);
		 ~GroupChatRoster();
	
		int Message(WokXMLTag *tag);
		int Activate(WokXMLTag *tag);
		int Tooltip(WokXMLTag *tag);
		int TooltipReset(WokXMLTag *tag);
		void Display();
	protected:
		std::string id;
		std::string room;
		std::string mainwid;
		std::string mynick;
		std::list <std::string> message;
	
};


#endif	//_GROUPCHATROSTER_H_

