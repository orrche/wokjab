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
// Class: Socks5
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Dec 19 21:47:49 2006
//

#ifndef _SOCKS5_H_
#define _SOCKS5_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class Socks5 : public WoklibPlugin
{
	public:
		Socks5(WLSignal *wls);
		 ~Socks5();
	
		int Connect(WokXMLTag *tag);
		int Host(WokXMLTag *tag);
	
		virtual std::string GetInfo() {return "SOCKS5 Handling";};
		virtual std::string GetVersion() {return VERSION;};
	protected:
		int sockid;
	
	
};


#endif	//_SOCKS5_H_

