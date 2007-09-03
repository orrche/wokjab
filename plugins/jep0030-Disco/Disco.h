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
// Class: Disco
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Wed Aug 24 12:22:12 2005
//

#ifndef _DISCO_H_
#define _DISCO_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace Woklib;


class Disco : public WoklibPlugin
{
	public:
		Disco(WLSignal *wls);
		 ~Disco();
	
		int RequestInfo(WokXMLTag *);
		int RequestItem(WokXMLTag *);
	
	
		virtual std::string GetInfo() {return "Disco GUI";};
		virtual std::string GetVersion() {return VERSION;};
	protected:
	
};


#endif	//_DISCO_H_

