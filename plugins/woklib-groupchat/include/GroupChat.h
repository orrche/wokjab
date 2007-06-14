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


#ifndef _GROUPCHAT_H_
#define _GROUPCHAT_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokLib.h>
#include <Woklib/WLSignal.h>

#include "MUCServer.h"

class GroupChat : public WoklibPlugin
{
	public:
		GroupChat(WLSignal *wls);
		 ~GroupChat();
		int Join(WokXMLTag *tag);
		int Part(WokXMLTag *tag);
		int Ban(WokXMLTag *tag);
		int Presence(WokXMLTag *tag);


		virtual std::string GetInfo() {return "MUC Groupchat";};
		virtual std::string GetVersion() {return VERSION;};
	protected:
		std::map <std::string, std::map <std::string, MUCServer> > servers;
};

#endif	//_GROUPCHAT_H_
