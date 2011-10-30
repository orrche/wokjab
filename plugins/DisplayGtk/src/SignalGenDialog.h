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
// Class: SignalGenDialog
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Sep 29 23:52:06 2006
//

#ifndef _SIGNALGENDIALOG_H_
#define _SIGNALGENDIALOG_H_

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/**
 * Class to send raw signals
 */
class SignalGenDialog: public WLSignalInstance
{
	public:
		SignalGenDialog(WLSignal *wls);
		 ~SignalGenDialog();
	
		static gboolean Delete(GtkWidget *widget, GdkEvent *event, SignalGenDialog *c);
		static gboolean SendButton(GtkButton *button, SignalGenDialog *c);
	protected:
		GladeXML *xml;
		GtkWidget *window;
};



#endif	//_SIGNALGENDIALOG_H_

