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
// Class: GtkPCommon
// Created by: Kent Gustavsson <nedo@gmail.com>
// Created on: Sun Aug 14 22:12:36 2005
//

#ifndef _GTKPCOMMON_H_
#define _GTKPCOMMON_H_

#include <Woklib/WokXMLTag.h>
#include <Woklib/WokLib.h>
#include <gtk/gtk.h>

using namespace Woklib;

class GtkPCommon
{
	public:
		GtkPCommon(WokXMLTag *c);
		virtual ~GtkPCommon();
	
		virtual GtkWidget * GetWidget();
		virtual void Save();
	protected:
		WokXMLTag *conf;		
};


#endif	//_GTKPCOMMON_H_

