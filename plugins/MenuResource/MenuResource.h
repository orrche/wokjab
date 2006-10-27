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
// Class: MenuResource
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 31 09:50:18 2006
//

#ifndef _MENURESOURCE_H_
#define _MENURESOURCE_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/**
 * Creates a sub menu for each resource that an individual jid has and calls 
 * to create a menu for each one_display_cache
 * Checks if the jid already has an resource to not loop in infinitie 
 */
class MenuResource : public WoklibPlugin
{
	public:
		MenuResource(WLSignal *wls);
		 ~MenuResource();
	
		int Menu(WokXMLTag *xml); /// Signal handler
	protected:
	
};


#endif	//_MENURESOURCE_H_

