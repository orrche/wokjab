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
// Class: GtkTimer
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu May 19 03:54:22 2005
//

#ifndef _GTKTIMER_H_
#define _GTKTIMER_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/**
 * To be used with none Gtk apps and plugins to get a timer with gtk apps and 
 * plugins there is no real point in using this over the gtk timer other then 
 * getting a nonstatic method maybe
 */
class GtkTimer:public WoklibPlugin
{
      public:
	GtkTimer (WLSignal * wls);
	~GtkTimer ();

	virtual std::string GetInfo() {return "Gtk Timer";};
	virtual std::string GetVersion() {return VERSION;};
	
		
	
	int Add (WokXMLTag * tag);

      protected:
		unsigned int id;

};


#endif //_GTKTIMER_H_
