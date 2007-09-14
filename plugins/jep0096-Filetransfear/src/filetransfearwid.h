/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef __REGISTER_SERVICE_WIDGET_H
#define __REGISTER_SERVICE_WIDGET_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include "map"

using namespace Woklib;

class jep96Widget : public WLSignalInstance
{
	public:
		jep96Widget(WLSignal *wls, WokXMLTag *xml, const std::string &lsid);
		~jep96Widget();

		int xdataresp(WokXMLTag *xml);
		int FileAuth(WokXMLTag *xml);
	
		void Activate();
		int Open(WokXMLTag *tag);
	
		static void Destroy (GtkWidget * widget, jep96Widget *c);
		static void ButtonPress (GtkButton *button, jep96Widget *c);
//		static void FileSelected
	protected:
		GtkWidget *window;
		GtkWidget *chooser;
		
		WokXMLTag *eventtag;
		WokXMLTag *origxml;
		std::string filename;
		std::string from;
		std::string id;
		std::string session;
		std::string lsid;
		bool requested;
};

#endif // __REGISTER_SERVICE_WIDGET_H
