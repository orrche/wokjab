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

#ifndef _WOKJABJIDINFOWIDGETMANAGER_H_
#define _WOKJABJIDINFOWIDGETMANAGER_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/** 
 * Signal handler for the JID Info widget 
 */
class WokJabJIDInfoWidgetManager : public WoklibPlugin
{
	public:
		WokJabJIDInfoWidgetManager(WLSignal *wls);
    	~WokJabJIDInfoWidgetManager();
	
		int NewWidget(WokXMLTag *tag);
		int InfoMenu(WokXMLTag *tag);
	

		virtual std::string GetInfo() {return "JID Info";};
		virtual std::string GetVersion() {return VERSION;};	
	protected:
	
};


#endif	//_WOKJABJIDINFOWIDGET_H_
