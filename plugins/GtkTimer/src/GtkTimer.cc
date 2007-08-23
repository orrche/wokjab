/***************************************************************************
 *  Copyright (C) 2005-2007  Kent Gustavsson <nedo80@gmail.com>
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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu May 19 03:54:22 2005
//

#include "GtkTimer.h"
#include "GtkTimerSession.h"

GtkTimer::GtkTimer (WLSignal * wls):
WoklibPlugin (wls)
{
	id = 0;
	EXP_SIGHOOK ("Woklib Timmer Add", &GtkTimer::Add, 999);

}


GtkTimer::~GtkTimer ()
{
}

int
GtkTimer::Add (WokXMLTag * tag)
{
	int time = atoi(tag->GetAttr ("time").c_str());

	if( tag->GetAttr("signal").empty() )
	{
		std::Stringstream str;
		
		str << "Woklib Timmer ID" << id;
		tag->AddAttr("signal", str.str() );
		
		id++;
	}
	
	new GtkTimerSession (wls, time, tag->GetAttr ("signal"));

	return 1;
}
