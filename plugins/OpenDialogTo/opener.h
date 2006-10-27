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
// Class: Opener
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 24 17:12:23 2006
//

#ifndef _OPENER_H_
#define _OPENER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

class Opener;

#include "dopener.h"

using namespace Woklib;

/** 
 * Signal handler for the dialog opener 
 */
class Opener : public WoklibPlugin
{
	public:
		Opener(WLSignal *wls);
		 ~Opener();

		virtual std::string GetInfo() {return "Open Dialog To";};
		virtual std::string GetVersion() {return VERSION;};
		
		int Menu(WokXMLTag *tag);
		int DialogOpener(WokXMLTag *tag);
		void DialogOpenerRemove(DOpener *wid);
	protected:
		std::list<DOpener *> window;
		
};


#endif	//_OPENER_H_

