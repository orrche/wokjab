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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu May 19 18:32:01 2005
//

#include "DisplayGtk.h"

#define STDOUT_DEBUG

DisplayGtk::DisplayGtk(WLSignal *wls) : WoklibPlugin(wls)
{
	widget = new DisplayWidget(wls);

	EXP_SIGHOOK("Display Error", &DisplayGtk::DispError, 1000);
	EXP_SIGHOOK("Display Socket", &DisplayGtk::DispSocket, 1000);
	EXP_SIGHOOK("Display Debug", &DisplayGtk::DispDebug, 1000);
	EXP_SIGHOOK("Display Message", &DisplayGtk::DispMsg, 1000);
	EXP_SIGHOOK("Display Signal", &DisplayGtk::DispSig, 1000);
	
	EXP_SIGHOOK("GetMenu", &DisplayGtk::Menu, 1000);
	EXP_SIGHOOK("Jabber GUI Debug Show", &DisplayGtk::ShowWidget, 1000);
}


DisplayGtk::~DisplayGtk()
{
	delete widget;
}

int
DisplayGtk::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("signal", "Jabber GUI Debug Show");
	item.AddAttr("name", "Debug");
	
	return true;
}

int
DisplayGtk::ShowWidget(WokXMLTag *tag)
{
	widget->Show();
	
	return true;
}

int
DisplayGtk::DispMsg(WokXMLTag *tag)
{
#ifdef STDOUT_DEBUG
	std::cout << "\033[22;34mM: " << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;
#endif
	widget->Message(tag);
	return 1;
}

int
DisplayGtk::DispDebug(WokXMLTag *tag)
{
#ifdef STDOUT_DEBUG
	std::cout << "\033[22;32mD: " << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;
#endif
	widget->Debug(tag);
	return 1;
}

int
DisplayGtk::DispError(WokXMLTag *tag)
{
#ifdef STDOUT_DEBUG
	std::cout << "\033[22;31mE: " << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;
#endif
	widget->Error(tag);
	return 1;
}

int
DisplayGtk::DispSocket(WokXMLTag *tag)
{
#ifdef STDOUT_DEBUG
	std::cout << "\033[01;34mS" << tag->GetAttr("type") << ":" << tag->GetFirstTag("body").GetBody() << "\033[22;37m" << std::endl;
#endif
	widget->InOut(tag);
	return 1;
}

int
DisplayGtk::DispSig(WokXMLTag *tag)
{
#ifdef STDOUT_DEBUG
	std::cout << "\033[22;35mSig: " << tag->GetAttr("name") << "\033[22;37m" << std::endl;
#endif
	widget->Signal(tag);
	return 1;
}

