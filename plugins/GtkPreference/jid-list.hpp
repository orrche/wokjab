/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifndef _JID_LIST_HPP_
#define _JID_LIST_HPP_


#include <gtk/gtk.h>
#include <glade/glade.h>

#include "GtkPCommon.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
using namespace Woklib;

class JIDList: public GtkPCommon 
{
public:
	JIDList(WokXMLTag *tag);
	~JIDList();
	
	GtkWidget * GetWidget();
	void Save();
	
	static void DataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time, JIDList *c);
	static gboolean DragMotion(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, JIDList *c);
	static void Remove(GtkButton *button, JIDList *c);
protected:
	GladeXML *gxml;	
private:

};

#endif // _JID_LIST_HPP_
