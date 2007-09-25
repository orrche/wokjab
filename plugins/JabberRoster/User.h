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
// Class: User
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Sep  2 14:39:43 2006
//

#ifndef _USER_H_
#define _USER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

class User;

#include "JabberSession.h"

using namespace Woklib;

class User : public WLSignalInstance
{
	public:
		User(WLSignal *wls, WokXMLTag *tag, JabberSession *ses);
		 ~User();

		void Show();
		void Hide();
		void UpdateEntry();
		void UpdateRow();
		
		void Update(WokXMLTag *tag);
		void AddEvent(WokXMLTag *tag);
		
		int RemoveEvent(WokXMLTag *tag);
		int Activate(WokXMLTag *tag);
		int Presence(WokXMLTag *tag);
		int Recheck(WokXMLTag *tag);
		int RightButton(WokXMLTag *tag);
		int Tooltip(WokXMLTag *tag);
		int TooltipReset(WokXMLTag *tag);
		int UpdateTicketEntries(WokXMLTag *tag);
		int UpdateXML(WokXMLTag *tag);
		int TickerUpdate(WokXMLTag *tag);
	protected:
		int tickerpos;
		std::list <std::string>::iterator currentticker;
	
		std::list <std::string> tickeritems;
		std::map <std::string, std::string> id;
		std::list <WokXMLTag *> Events;
		bool visible;
		JabberSession *ses;
		std::string name, icon, avatar, showmsg, statusmsg, jid;
		WokXMLTag *usertag;
};


#endif	//_USER_H_

