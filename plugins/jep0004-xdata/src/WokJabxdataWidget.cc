/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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
#include "../include/WokJabxdataWidget.h"
#include "../include/xdataunknown.h"
#include "../include/xdatamultitext.h"
#include "../include/xdatasingletext.h"
#include "../include/xdatafixed.h"
#include "../include/xdatasingleprivate.h"
#include "../include/xdatabool.h"
#include "../include/xdatahidden.h"
#include "../include/xdatasinglelist.h"

#include <iostream>
#include <sstream>

using std::cerr;
using std::endl;
using std::cout;

/* Hmm those widgets are prime candidates for plugins */

WokJabxdataWidget::WokJabxdataWidget(WLSignal *wls, WokXMLTag *tag_iq) : WLSignalInstance(wls),
replsig(tag_iq->GetAttr("signal"))
{
	WokXMLTag* tag_title;
	WokXMLTag* tag_instructions;
	WokXMLTag *tag_x = &tag_iq->GetFirstTag("x");
	std::list <WokXMLTag*>::iterator iter;
	
	tag_title = &tag_x->GetFirstTag("title");
	tag_instructions = &tag_x->GetFirstTag("instructions");
	main_hbox = gtk_vbox_new(false, false);
	
	if ( tag_iq->GetTagList("plug").empty())
	{
		GtkWidget *hbox;
		
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		ok_button = gtk_button_new_with_label("Ok");
		cancel_button = gtk_button_new_with_label("Cancel");
		hbox = gtk_hbox_new(true, 0);
		
		gtk_box_pack_start(GTK_BOX(hbox), ok_button, false , false , 0);
		gtk_box_pack_start(GTK_BOX(hbox), cancel_button, false , false , 0);
		gtk_box_pack_end(GTK_BOX(main_hbox), hbox, false, false ,0);
		
		g_signal_connect (G_OBJECT (ok_button), "clicked",
			  G_CALLBACK (WokJabxdataWidget::OkButton),
			  this);
		g_signal_connect (G_OBJECT (cancel_button), "clicked",
			  G_CALLBACK (WokJabxdataWidget::CancelButton),
			  this);
	}
	else
	{
		window = gtk_plug_new(0);
		std::stringstream str;
		str << gtk_plug_get_id(GTK_PLUG(window));
		tag_iq->GetFirstTag("plug").AddAttr("id", str.str());
	}
	
	//GtkWidget *scroll_win = gtk_scrolled_window_new(NULL, NULL);
	
	//gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scroll_win), main_hbox);
	gtk_container_add(GTK_CONTAINER(window), main_hbox);
	
	if(tag_title)
		gtk_window_set_title(GTK_WINDOW(window), tag_title->GetBody().c_str());
	if(tag_instructions)
	{
		GtkWidget *label_instructions;
		label_instructions = gtk_label_new(tag_instructions->GetBody().c_str());
		gtk_label_set_line_wrap(GTK_LABEL(label_instructions), true);
		gtk_box_pack_start(GTK_BOX(main_hbox), label_instructions, false, false, 2); 
	}

	for(iter = tag_x->GetTagList("field").begin(); iter!= tag_x->GetTagList("field").end(); iter++)
	{
		std::string type;
		GtkWidget *box;
		
		type = (*iter)->GetAttr("type");
		box = gtk_vbox_new(false, false);
		gtk_box_pack_start(GTK_BOX(main_hbox), box, false, false, 2);
		
		if(type == "text-multi")
			current_widget = new xdatamultitext(wls, *iter , box);
		else if(type == "text-single")
			current_widget = new xdatasingletext(wls, *iter, box);
		else if(type == "fixed")
			current_widget = new xdatafixed(wls, *iter, box);
		else if(type == "text-private")
			current_widget = new xdatasingleprivate(wls, *iter, box);
		else if(type == "boolean")
			current_widget = new xdatabool(wls,*iter, box);
		else if(type == "hidden")
			current_widget = new xdatahidden(wls, *iter, box);
		else if(type == "list-single")
			current_widget = new xdatasinglelist(wls, *iter, box);
		else 
			current_widget = new xdataunknown(wls, *iter, box);
		
		if(current_widget)
			widget_list.push_back(current_widget); 
	}

	g_signal_connect (G_OBJECT (window), "destroy",
				G_CALLBACK (WokJabxdataWidget::Destroy), this);
				
	gtk_widget_show_all(window);
	
	EXP_SIGHOOK("Jabber jabber:x:data Get " + tag_iq->GetAttr("id"), &WokJabxdataWidget::GetData, 1000);
}


WokJabxdataWidget::~WokJabxdataWidget()
{	
	std::list < xdatabase * >::iterator iter;
	iter = widget_list.begin();
	
	for( iter = widget_list.begin() ; iter != widget_list.end() ; iter++)
		delete ( *iter);		
}

void
WokJabxdataWidget::Destroy(GtkWidget *widget, WokJabxdataWidget *c)
{
	delete c;
}

void
WokJabxdataWidget::CancelButton (GtkWidget * widget, WokJabxdataWidget *c)
{
	WokXMLTag repltag(NULL, "data");
	WokXMLTag &xtag = repltag.AddTag("x");
	xtag.AddAttr("xmlns", "jabber:x:data");
	xtag.AddAttr("type", "cancel");
	
	c->wls->SendSignal(c->replsig, &repltag);
	gtk_widget_destroy (c->window);
}
void
WokJabxdataWidget::OkButton (GtkWidget * widget, WokJabxdataWidget *c)
{
	// Need to cache this because get data deletes the class ;) 
	std::string repsig = c->replsig;
	
	WokXMLTag repltag(NULL, "data");
	c->GetData(&repltag);
	
	c->wls->SendSignal(c->replsig, &repltag);
}

int
WokJabxdataWidget::GetData(WokXMLTag *repltag)
{
	std::list<xdatabase *>::iterator iter;

	WokXMLTag &xtag = repltag->AddTag("x");
	xtag.AddAttr("xmlns", "jabber:x:data");
	xtag.AddAttr("type", "submit");
	
	for( iter = widget_list.begin() ; iter != widget_list.end() ; iter++)
	{
		if((*iter)->GetVar() != "")
		{
			WokXMLTag &fieldtag = xtag.AddTag("field");
			fieldtag.AddAttr("var", (*iter)->GetVar());
			fieldtag.AddTag("value").AddText((*iter)->GetData());
		}
	}
	
	gtk_widget_destroy (window);
	
	return 1;
}
