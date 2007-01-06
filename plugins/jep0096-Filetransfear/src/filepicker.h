/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <nedo80@gmail.com>
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
// Class: filepicker
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug 23 19:51:51 2005
//

#ifndef _FILEPICKER_H_
#define _FILEPICKER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class filepicker : public WLSignalInstance
{
	public:
		filepicker(WLSignal *wls, const std::string & session, const std::string & to);
		 ~filepicker();

		static void Destroy (GtkWidget * widget, filepicker *c);
		static void ButtonPress (GtkButton *button, filepicker *c);
	protected:
		GladeXML *gxml;
		
		std::string session;
		std::string to;
};


#endif	//_FILEPICKER_H_

