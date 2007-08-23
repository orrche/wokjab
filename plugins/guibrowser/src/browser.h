/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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

#ifndef __BROWSER_H
#define __BROWSER_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>
#include <gtk/gtk.h>
#include <list>

class Browser;

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "browserwid.h"

using namespace Woklib;
/**
 * Browser signal handler 
 */
class Browser : public WoklibPlugin
{
	public:
		Browser(WLSignal *wls);
		~Browser();

		virtual std::string GetInfo() {return "Browser";};
		virtual std::string GetVersion() {return VERSION;};
			
		int MainMenu(WokXMLTag *xml);
		int BrowserWid(WokXMLTag *xml);
		int JidMenu(WokXMLTag *xml);
	
		void BrowserWidRemove(BrowserWidget *wid);
	protected:
		std::list<BrowserWidget *> window;

};

#endif // __BROWSER_H
