/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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


#include "Win32GtkPlug.h"
#include <iostream>

GtkWidget*  gtk_plug_new(GdkNativeWindow socket_id)
{
    g_assert(socket_id==0);

    return gtk_vbox_new(FALSE, FALSE);
}

GdkNativeWindow gtk_plug_get_id (GtkPlug *plug)
{
    return (int)(plug);
}

GtkWidget*
gtk_socket_new(void)
{
    return gtk_vbox_new(FALSE, FALSE);
}

void
gtk_socket_add_id (GtkSocket *socket_, GdkNativeWindow window_id)
{
    gtk_box_pack_start(GTK_BOX(socket_), GTK_WIDGET(window_id), TRUE, TRUE, 0);
    gtk_widget_show_all(GTK_WIDGET(window_id));
    gtk_widget_show_all(GTK_WIDGET(socket_));
}

GtkType
gtk_socket_get_type (void)
{
    return gtk_vbox_get_type();
}

GtkType
gtk_plug_get_type (void)
{
    return gtk_vbox_get_type();
}

