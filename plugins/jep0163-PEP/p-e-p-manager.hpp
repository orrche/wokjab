/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent 2007 <Gustavsson>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifndef _P_E_P_MANAGER_HPP_
#define _P_E_P_MANAGER_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace Woklib;

class PEPManager;

#include "p-e-p-widget.hpp"

class PEPManager: public WoklibPlugin 
{
public:
	PEPManager(WLSignal *wls);
	~PEPManager();
	
	int Menu(WokXMLTag *tag);
	int DialogOpener(WokXMLTag *tag);
	void DialogOpenerRemove( PEP_Widget *gmw );
			
	
	virtual std::string GetInfo() {return "PEP Manager";};
	virtual std::string GetVersion() {return VERSION;};
protected:
	std::list<PEP_Widget *> window;
protected:

private:

};

#endif // _P_E_P_MANAGER_HPP_
