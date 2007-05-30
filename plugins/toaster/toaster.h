/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Toaster
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Dec 10 04:24:23 2005
//

#ifndef _TOASTER_H_
#define _TOASTER_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ToasterWindow.h"

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>

using namespace Woklib;
/** 
 * A not finished toaster plugin. Kent stop sitting on your hands 
 */
class Toaster : public WoklibPlugin
{
	public:
		Toaster(WLSignal *wls);
		 ~Toaster();
	
		int AddJIDEvent(WokXMLTag *tag);
		int DisplayMSG(WokXMLTag *tag);
		int ReadConfig(WokXMLTag *tag);
		int RemoveMSG(WokXMLTag *tag);
	
		bool GetXWorkArea(GdkRectangle *rect);
		
			
		virtual std::string GetInfo() {return "Toaster";};
		virtual std::string GetVersion() {return VERSION;};
	protected:
		GdkRectangle rect_workspace;
		std::list <ToasterWindow*> twlist;
		int twid;
		WokXMLTag *config;
};


#endif	//_TOASTER_H_

