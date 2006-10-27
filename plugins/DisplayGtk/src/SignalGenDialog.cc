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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Sep 29 23:52:06 2006
//

#include "SignalGenDialog.h"
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <sstream>


SignalGenDialog::SignalGenDialog(WLSignal *wls) : WLSignalInstance(wls)
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/displaygtk.signalgen.glade", NULL, NULL);

	window = glade_xml_get_widget (xml, "signalgenwindow");
	
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (SignalGenDialog::Delete), this);	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "SendButton")), "clicked",
			G_CALLBACK (SignalGenDialog::SendButton), this);
}


SignalGenDialog::~SignalGenDialog()
{
	if ( xml )
	{
		g_object_unref(xml);
		xml = NULL;
	}
}

gboolean 
SignalGenDialog::Delete(GtkWidget *widget, GdkEvent *event, SignalGenDialog *c)
{
	delete c;
	return false;
}

gboolean
SignalGenDialog::SendButton(GtkButton *button, SignalGenDialog *c)
{
	GtkWidget *text = glade_xml_get_widget (c->xml, "sigdata");
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	std::stringstream str;
	GtkTextIter start_iter, end_iter;
	std::string signal;
	
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
		
		
	WokXMLTag *sendingtag;
	WokXMLTag tag(NULL,"message");
	gchar *buftxt;
	buftxt = gtk_text_buffer_get_text(buffer,&start_iter, &end_iter, false);
	str << buftxt;
	str >> tag;
	
	if ( tag.GetTags().empty() )
		return true;
	
	signal = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (c->xml, "sigentry")));
	if ( signal.empty() )
	{
		if ( (*tag.GetTags().begin())->GetName() == "signal")
		{
			signal = tag.GetFirstTag("signal").GetAttr("name");
			if ( tag.GetFirstTag("signal").GetTags().empty() )
				return true;
			sendingtag = *tag.GetFirstTag("signal").GetTags().begin();
		}
		else
			return true;
	}
	else
		sendingtag = *tag.GetTags().begin();

	std::cout << "::" << std::flush << signal << std::endl;
	std::cout << "::" << std::flush << sendingtag << std::endl;
	c->wls->SendSignal(signal, sendingtag);
	
	gtk_widget_destroy(c->window);
	return false;
}
