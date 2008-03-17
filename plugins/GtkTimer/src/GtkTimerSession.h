/***************************************************************************
 *  Copyright (C) 2005-2008  Kent Gustavsson <nedo80@gmail.com>
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
// Class: GtkTimerSession
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu May 19 04:24:35 2005
//

#ifndef _GTKTIMERSESSION_H_
#define _GTKTIMERSESSION_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>

#include <string>

using namespace Woklib;

class GtkTimerSession:public WLSignalInstance
{
public:
	GtkTimerSession (WLSignal * wls, int timeout, WokXMLTag *tag);
	 ~GtkTimerSession ();

	static gboolean Exec (GtkTimerSession * c);

protected:
	WokXMLTag origxml;
	int timeout;
	guint gtimid;

};


#endif //_GTKTIMERSESSION_H_
