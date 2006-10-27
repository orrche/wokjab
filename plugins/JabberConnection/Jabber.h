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


#ifndef _JABBER_H_
#define _JABBER_H_

class Jabber;

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "Connection.h"

using namespace Woklib;

class Jabber : public WoklibPlugin
{
	public:
		Jabber(WLSignal *wls);
		 ~Jabber();
	
		std::string connect(std::string server, std::string sername, std::string password, std::string resource, int port, int type);
		int SignalDisconnect(WokXMLTag *tag);

		int SignalConnect(WokXMLTag *tag);
		int SendXML(WokXMLTag *tag);
		int GetSessions(WokXMLTag *tag);
		int ConnectionLost(WokXMLTag *tag);

	protected:
		int session_nr;
		std::map<std::string, Connection *> connections;
};


#endif	//_JABBER_H_

