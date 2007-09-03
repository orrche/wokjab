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
// Class: EntCap
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Aug 26 22:37:01 2005
//

#ifndef _ENTCAP_H_
#define _ENTCAP_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace Woklib;

/**
 * Caputres presences and adds caps ( JEP115 ) 
 * not even sure I'm doing this right
 * Was done to fool google client that it could use voice with me 
 */
class EntCap : public WoklibPlugin
{
	public:
		EntCap(WLSignal *wls);
		 ~EntCap();
	
	
		virtual std::string GetInfo() {return "EntCap";};
		virtual std::string GetVersion() {return VERSION;};
		int Presence(WokXMLTag *tag);
	protected:
		
		
};


#endif	//_ENTCAP_H_

