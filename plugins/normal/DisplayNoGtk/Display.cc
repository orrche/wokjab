/***************************************************************************
 *  Copyright (C) 2005-2010  Kent Gustavsson <nedo80@gmail.com>
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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu May 19 18:32:01 2005
//

#include "Display.h"

#define STDOUT_DEBUG

Display::Display(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Display Error", &Display::DispError, 1000);
	EXP_SIGHOOK("Display Socket", &Display::DispSocket, 1000);
	EXP_SIGHOOK("Display Debug", &Display::DispDebug, 1000);
	EXP_SIGHOOK("Display Message", &Display::DispMsg, 1000);
	EXP_SIGHOOK("Display Signal", &Display::DispSig, 1000);
}


Display::~Display()
{
}

int
Display::DispMsg(WokXMLTag *tag)
{

	std::cout << "\033[22;34mM: " << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;

	return 1;
}

int
Display::DispDebug(WokXMLTag *tag)
{
	std::cout << "\033[22;32mD: " << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;

	return 1;
}

int
Display::DispError(WokXMLTag *tag)
{
	std::cout << "\033[22;31mE: " << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;

	return 1;
}

int
Display::DispSocket(WokXMLTag *tag)
{
	std::cout << "\033[01;34mS" << tag->GetAttr("type") << ":" << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;

	return 1;
}

int
Display::DispSig(WokXMLTag *tag)
{
	for ( int i = atoi(tag->GetAttr("level").c_str()); i ; i-- )
		std::cout << "  ";
	std::cout << "\033[22;35mSig: " << tag->GetAttr("name") << "\033[22;37m" << std::endl;

	return 1;
}

