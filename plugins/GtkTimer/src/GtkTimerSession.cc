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
// Class: GtkTimmerSession
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu May 19 04:24:35 2005
//


#include "GtkTimerSession.h"

GtkTimerSession::GtkTimerSession (WLSignal * wls, int timeout, WokXMLTag *tag):
WLSignalInstance (wls),
origxml (*tag)
{
	gtimid = g_timeout_add (timeout, (gboolean (*)(void *)) (GtkTimerSession::Exec), this);
}


GtkTimerSession::~GtkTimerSession ()
{
	if ( gtimid )
		g_source_remove(gtimid);
}

gboolean 
GtkTimerSession::Exec (GtkTimerSession * c)
{
	WokXMLTag tag (c->origxml.GetFirstTag("data"));
	if (c->wls->SendSignal (c->origxml.GetAttr("signal"), tag) == 0)
	{
		c->wls->SendSignal ("Woklib Timmer Remove", c->origxml);
		return FALSE;
	}
	
	if(!tag.GetAttr("stop").empty())
	{
		c->wls->SendSignal ("Woklib Timmer Remove", c->origxml);
		return FALSE;
	}
		
	return TRUE;
}
