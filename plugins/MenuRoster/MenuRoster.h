/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <oden@gmx.net>
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
// Class: MenuRoster
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 31 18:41:35 2006
//

#ifndef _MENUROSTER_H_
#define _MENUROSTER_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/**
 * Creates a roster in the menu thanks to blackdog for the idea 
 */
class MenuRoster : public WoklibPlugin
{
	public:
		MenuRoster(WLSignal *wls);
		 ~MenuRoster();

		virtual std::string GetInfo() {return "Menu Roster";};
		virtual std::string GetVersion() {return VERSION;};

		int CreateRoster(WokXMLTag *tag);
		void AddUserEntry(WokXMLTag *pos, WokXMLTag *usertag, std::string session);
	protected:
		
	
};


#endif	//_MENUROSTER_H_

