/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
 ****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with main.c; if not, write to:
 *            The Free Software Foundation, Inc.,
 *            59 Temple Place - Suite 330,
 *            Boston,  MA  02111-1307, USA.
 */


//
// Class: DisplayGtk
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu May 19 18:32:01 2005
//

#ifndef _DISPLAYGTK_H_
#define _DISPLAYGTK_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "DisplayWidget.h"

using namespace Woklib;
/**
 * Class to handle the signals for the Debug/Error/Message display also print 
 * messages to the terminal
 */
class DisplayGtk : public WoklibPlugin
{
	public:
		DisplayGtk(WLSignal *wls);
		 ~DisplayGtk();
		
		virtual std::string GetInfo() {return "Error and message display";};
		virtual std::string GetVersion() {return "0.0.1";};
	
	int DispError(WokXMLTag *tag);
	int DispMsg(WokXMLTag *tag);
	int DispDebug(WokXMLTag *tag);
	int DispSocket(WokXMLTag *tag);
	int DispSig(WokXMLTag *tag);
	int Menu(WokXMLTag *tag);
	int ShowWidget(WokXMLTag *tag);
		// DisplayGtk interface
	
		// TODO: add member function declarations...
	
	protected:
		DisplayWidget *widget;
};


#endif	//_DISPLAYGTK_H_

