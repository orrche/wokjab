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
// Class: GtkPString
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Aug 14 19:13:38 2005
//

#ifndef _GTKPSTRING_H_
#define _GTKPSTRING_H_

#include "GtkPCommon.h"

using namespace Woklib;

class GtkPString : public GtkPCommon
{
	public:
		GtkPString(WokXMLTag *tag);
		 ~GtkPString();
		
		GtkWidget *GetWidget();
		void Save();
	protected:
		GtkWidget *entry;
		GtkWidget *hbox;
};


#endif	//_GTKPSTRING_H_

