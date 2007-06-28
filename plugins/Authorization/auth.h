/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Auth
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug  9 12:59:33 2005
//

#ifndef _AUTH_H_
#define _AUTH_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/**
 * Class for requesting presence subscription from menu
 */
class Auth : public WoklibPlugin
{
	public:
		Auth(WLSignal *wls);
		 ~Auth();

		virtual std::string GetInfo() {return "Request Authentication";};
		virtual std::string GetVersion() {return VERSION;};

		int Menu(WokXMLTag *tag); 																	/// Menu creator
		int ReqAuthAction(WokXMLTag *tag); 													/// Method that actually does something
	protected:
		
};


#endif	//_AUTH_H_

