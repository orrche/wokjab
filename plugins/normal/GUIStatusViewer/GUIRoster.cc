/***************************************************************************
 *  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>

#include "GUIRoster.h"

#include <iostream>
#include <sstream>

GUIRoster::GUIRoster (WLSignal * wls):WoklibPlugin (wls)
{				
	hbb = gtk_hbox_new(FALSE, FALSE );
	saveconfig = true;

	togg1 = gtk_toggle_button_new();
	togg2 = gtk_toggle_button_new();
	togg3 = gtk_toggle_button_new();
	
	gtk_box_pack_start( GTK_BOX(hbb), togg1, FALSE, FALSE, FALSE);
	gtk_box_pack_start( GTK_BOX(hbb), togg2, FALSE, FALSE, FALSE);
	gtk_box_pack_start( GTK_BOX(hbb), togg3, FALSE, FALSE, FALSE);
	
	EXP_SIGHOOK("Jabber GUI Roster View", &GUIRoster::View, 1);
	EXP_SIGHOOK("Jabber GUI GetIcon", &GUIRoster::GetIcon, 1);
	EXP_SIGHOOK("Jabber GUI Roster GetJIDMenu", &GUIRoster::DeleteMenu, 1);
	EXP_SIGHOOK("Jabber GUI Roster DeleteJID", &GUIRoster::DeleteAction, 1);
	
	icons["away"] = PACKAGE_DATA_DIR"/wokjab/away.png";
	icons["dnd"] = PACKAGE_DATA_DIR"/wokjab/dnd.png";
	icons["xa"] = PACKAGE_DATA_DIR"/wokjab/xa.png";
	
	gtk_container_add (GTK_CONTAINER (togg1), gtk_image_new_from_file(std::string(PACKAGE_DATA_DIR"/wokjab/offline.png").c_str()));
	gtk_container_add (GTK_CONTAINER (togg2), gtk_image_new_from_file(icons["dnd"].c_str()));
	gtk_container_add (GTK_CONTAINER (togg3), gtk_image_new_from_file(icons["xa"].c_str()));
	
	g_signal_connect (G_OBJECT (togg1), "toggled",
		      G_CALLBACK (GUIRoster::ToggleButton), this);
	g_signal_connect (G_OBJECT (togg2), "toggled",
		      G_CALLBACK (GUIRoster::ToggleButton), this);
	g_signal_connect (G_OBJECT (togg3), "toggled",
		      G_CALLBACK (GUIRoster::ToggleButton), this);

	GtkTooltips *tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, togg1, "Show offline contacts in the roster", "Show offline contacts in the roster");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, togg2, "Show DND contacts in the roster", "Show DND contacts in the roster");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, togg3, "Show XA contacts in the roster", "Show XA contacts in the roster");
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /main/window/roster_visibility", &GUIRoster::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window/roster_visibility");
	wls->SendSignal("Config XML Trigger", &conftag);
	

	plugwid = gtk_plug_new(0);
	gtk_container_add(GTK_CONTAINER(plugwid), hbb);	
	gtk_widget_show_all(plugwid);

	std::stringstream buf;
	WokXMLTag contag(NULL, "addwidget");
	buf << gtk_plug_get_id(GTK_PLUG(plugwid));
	WokXMLTag &widtag = contag.AddTag("widget");
	widtag.AddAttr("id", buf.str());
	widtag.AddAttr("fill", "false");
	widtag.AddAttr("expand", "false");
	widtag.AddAttr("pos", "20");
		
	if( wls->SendSignal("GUI Window AddWidget",&contag) )
	{
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(plugwid));
		EXP_SIGHOOK(sig.str(), &GUIRoster::Close, 500);
	}
	else
	{
		EXP_SIGHOOK("GUI Window Init", &GUIRoster::WindowInit, 20);

	}
}

GUIRoster::~GUIRoster ()
{	
	EXP_SIGUNHOOK("Config XML Change /main/window/roster_visibility", &GUIRoster::ReadConfig, 500);
	
	SaveConfig();

	if( plugwid )
		gtk_widget_destroy(plugwid);

}

int
GUIRoster::WindowInit(WokXMLTag *tag)
{
		
	std::stringstream buf;
	WokXMLTag contag(NULL, "addwidget");
	buf << gtk_plug_get_id(GTK_PLUG(plugwid));
	WokXMLTag &widtag = contag.AddTag("widget");
	widtag.AddAttr("id", buf.str());
	widtag.AddAttr("fill", "false");
	widtag.AddAttr("expand", "false");
	widtag.AddAttr("pos", "20");
		
	if( wls->SendSignal("GUI Window AddWidget",&contag) )
	{
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(plugwid));
		EXP_SIGHOOK(sig.str(), &GUIRoster::Close, 500);
	}

	return 1;
}

int
GUIRoster::Close(WokXMLTag *tag)
{
	delete this;
	return 1;
}

void
GUIRoster::ToggleButton(GtkWidget *widget, GUIRoster *c)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
		c->bstate[widget] = TRUE;
	else 
		c->bstate[widget] = FALSE;
		
	if ( c->saveconfig ) 
		c->SaveConfig();
	WokXMLTag empty(NULL, "empty");
	c->wls->SendSignal("Jabber Roster Recheck", empty);
}

int
GUIRoster::ReadConfig (WokXMLTag *tag)
{
	saveconfig = false;
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	if( config->GetFirstTag("show").GetFirstTag("offline").GetAttr("data") != "false" )
		bstate[togg1] = true;
	else
		bstate[togg1] = false;
	if( config->GetFirstTag("show").GetFirstTag("dnd").GetAttr("data") != "false" )
		bstate[togg2] = true;
	else
		bstate[togg2] = false;
	if( config->GetFirstTag("show").GetFirstTag("xa").GetAttr("data") != "false" )
		bstate[togg3] = true;
	else
		bstate[togg3] = false;
	
	tag->GetFirstTag("config").GetFirstTag("show").GetFirstTag("xa").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("show").GetFirstTag("dnd").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("show").GetFirstTag("offline").AddAttr("type", "bool");
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(togg1), bstate[togg1]);			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(togg2), bstate[togg2]);			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(togg3), bstate[togg3]);

	saveconfig = true;
	return 1;
}

void
GUIRoster::SaveConfig()
{
	if(bstate[togg1])
		config->GetFirstTag("show").GetFirstTag("offline").AddAttr("data", "true");
	else
		config->GetFirstTag("show").GetFirstTag("offline").AddAttr("data", "false");
	if(bstate[togg2])
		config->GetFirstTag("show").GetFirstTag("dnd").AddAttr("data", "true");
	else
		config->GetFirstTag("show").GetFirstTag("dnd").AddAttr("data", "false");
	if(bstate[togg3])
		config->GetFirstTag("show").GetFirstTag("xa").AddAttr("data", "true");
	else
		config->GetFirstTag("show").GetFirstTag("xa").AddAttr("data", "false");
	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window/roster_visibility");
	conftag.AddTag(config);
	
	wls->SendSignal("Config XML Store", &conftag);
}

int
GUIRoster::View(WokXMLTag *tag)
{
	if(tag->GetFirstTag("item").GetTagList("resource").begin() == tag->GetFirstTag("item").GetTagList("resource").end())
		wls->SendSignal("Jabber Roster GetResource",tag);
	
	std::list <WokXMLTag*>::iterator tagiter;
	
	for( tagiter = tag->GetTagList("item").begin() ; tagiter != tag->GetTagList("item").end() ; tagiter++)
	{
		std::list <WokXMLTag*>::iterator riter;
		for( riter = (*tagiter)->GetTagList("resource").begin() ; riter != (*tagiter)->GetTagList("resource").end() ; riter++)
		{
			if( (*tagiter)->GetAttr("type") == "unavailable")
			{
				if( bstate[togg1] )
					(*riter)->AddAttr("view", "true");
			}
			else if ( (*riter)->GetFirstTag("show").GetBody() == "xa")
			{
				if( bstate[togg3] )
					(*riter)->AddAttr("view", "true");
			}
			else if ( (*riter)->GetFirstTag("show").GetBody() == "dnd")
			{
				if( bstate[togg2] )
					(*riter)->AddAttr("view", "true");
			}
			else
				(*riter)->AddAttr("view", "true");
		}
		if( (*tagiter)->GetTagList("resource").begin() == (*tagiter)->GetTagList("resource").end() )
		{
			if( bstate[togg1] )
				(*tagiter)->AddAttr("view", "true");
		}
		else
			(*tagiter)->AddAttr("view",(*tagiter)->GetFirstTag("resource").GetAttr("view"));
	}

	return 1;	
}

int
GUIRoster::GetIcon(WokXMLTag *tag)
{	
	if(tag->GetFirstTag("item").GetTagList("resource").begin() == tag->GetFirstTag("item").GetTagList("resource").end())
		wls->SendSignal("Jabber Roster GetResource",tag);
	
	std::list <WokXMLTag*>::iterator tagiter;
	for( tagiter = tag->GetTagList("item").begin() ; tagiter != tag->GetTagList("item").end() ; tagiter++)
	{
		std::list <WokXMLTag*>::iterator riter;
		for( riter = (*tagiter)->GetTagList("resource").begin() ; riter != (*tagiter)->GetTagList("resource").end() ; riter++)
		{
			if( (*tagiter)->GetAttr("type") == "unavailable")
				(*riter)->AddAttr("icon", PACKAGE_DATA_DIR"/wokjab/offline.png");
			else if ( (*riter)->GetFirstTag("show").GetBody() != "")
				(*riter)->AddAttr("icon", icons[(*riter)->GetFirstTag("show").GetBody()]);
			else
				(*riter)->AddAttr("icon", PACKAGE_DATA_DIR"/wokjab/online.png");
		}
		if( (*tagiter)->GetTagList("resource").begin() == (*tagiter)->GetTagList("resource").end() )
		{
			(*tagiter)->AddAttr("icon", PACKAGE_DATA_DIR"/wokjab/offline.png");
		}
		else
			(*tagiter)->AddAttr("icon",(*tagiter)->GetFirstTag("resource").GetAttr("icon"));
	}
	
	return 1;
}

int
GUIRoster::DeleteMenu(WokXMLTag *xml)
{
	WokXMLTag *item;
	item = & xml->AddTag("item");
	item->AddAttr("name", "Delete");
	item->AddAttr("signal", "Jabber GUI Roster DeleteJID");

	return 1;
}

int
GUIRoster::DeleteAction(WokXMLTag *tag)
{
	std::string xml_data;	
	
	xml_data = std::string("<iq type=\"set\"><query xmlns=\"jabber:iq:roster\" >")
			+ "<item subscription=\"remove\" jid=\"" 
			+ tag->GetAttr("jid") + "\" /></query></iq>";
	
	WokXMLTag msgtag(NULL,"message");
	msgtag.Add(xml_data);
	msgtag.AddAttr("session", tag->GetAttr("session"));
	wls->SendSignal("Jabber XML Send", &msgtag);
	
	return true;
}
