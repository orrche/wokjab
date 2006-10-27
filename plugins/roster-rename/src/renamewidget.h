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

#ifndef __RENAMEWIDGET_H
#define __RENAMEWIDGET_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "iostream"
#include <gtk/gtk.h>
#include "string"

using namespace Woklib;

/** 
 * The really pretty widget that is displayed when changing jid's nick
 */
class RenameWidget
{
	public:
		RenameWidget(WLSignal *wls, const std::string &jid, const std::string& session);
		~RenameWidget();
		
		static void Destroy(GtkWidget * widget, RenameWidget *c);
		static void ButtonPress(GtkWidget *widget, RenameWidget *c);
	protected:
		std::string jid;
		std::string session;
		WLSignal *wls;
		GtkWidget *alias_entry;
		GtkWidget *window;
		GtkWidget *sessionchooser;
	
	
};



#endif // __RENAMEWIDGET_H

