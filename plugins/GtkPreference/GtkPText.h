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
// Class: GtkPText
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Aug 14 19:34:05 2005
//

#ifndef _GTKPTEXT_H_
#define _GTKPTEXT_H_

#include "GtkPCommon.h"

using namespace Woklib;

class GtkPText : public GtkPCommon
{
	public:
		GtkPText(WokXMLTag *tag);
		 ~GtkPText();
	
		GtkWidget * GetWidget();
		void Save();
	protected:
		GtkWidget *vbox;
		GtkWidget *textview;
};


#endif	//_GTKPTEXT_H_

