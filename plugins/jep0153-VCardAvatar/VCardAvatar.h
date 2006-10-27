/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: VCardAvatar
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Aug 28 23:42:20 2006
//

#ifndef _VCARDAVATAR_H_
#define _VCARDAVATAR_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <map>
#include "User.h"

using namespace Woklib;

class VCardAvatar : public WoklibPlugin
{
	public:
		VCardAvatar(WLSignal *wls);
		 ~VCardAvatar();
	
		int SendPresence(WokXMLTag *tag);
		int SetMy(WokXMLTag *tag);
		int Presence(WokXMLTag *tag);
		int vcard(WokXMLTag *tag);
		int GetIcon(WokXMLTag *tag);
		int result(WokXMLTag *tag);
	protected:
		std::string Base64encode(const unsigned char *buf, int len);
		
		std::map <std::string, User *> user;
		std::string myhash;
		std::string signal;
		bool ready;
};


#endif	//_VCARDAVATAR_H_

