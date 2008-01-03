/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Tooltip
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Feb 12 20:06:04 2006
//

#ifndef _TOOLTIP_H_
#define _TOOLTIP_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class Tooltip : public WoklibPlugin
{
	public:
		Tooltip(WLSignal *wls);
		 ~Tooltip();
	
		int Reset(WokXMLTag &tag);
		int Set(WokXMLTag &tag);
		static gboolean DispWindow (Tooltip * c);
		static gboolean Expose(GtkWidget *widget, GdkEventExpose *event, Tooltip *c);

		virtual std::string GetInfo() {return "Tooltip";};
		virtual std::string GetVersion() {return VERSION;};
	protected:
		WokXMLTag *currenttag;
		GladeXML *xml;
		guint id;
};


#endif	//_TOOLTIP_H_

