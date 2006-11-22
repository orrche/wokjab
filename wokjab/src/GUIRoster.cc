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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>

#include "include/GUIRoster.h"

// Addons .. should be plugins maybe
#include "include/GUIAddJIDWidget.h"
#include "include/GUIPresentReqWidget.h"


#include <iostream>
#include <sstream>

using std::cout;
using std::endl;


GUIRoster::GUIRoster (WLSignal * wls):WLSignalInstance (wls)
{				
	hbb = gtk_hbox_new(false, false );
	
	togg1 = gtk_toggle_button_new();
	togg2 = gtk_toggle_button_new();
	togg3 = gtk_toggle_button_new();
	
	gtk_box_pack_start( GTK_BOX(hbb), togg1, false, false, false);
	gtk_box_pack_start( GTK_BOX(hbb), togg2, false, false, false);
	gtk_box_pack_start( GTK_BOX(hbb), togg3, false, false, false);
	
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

	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /main/window", &GUIRoster::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	// Initiating add ins .. maybe should be plugins..
	new GUIAddJIDWidget(wls);
	new GUIPresentReqWidget(wls);
	
	
	GtkWidget *plugwid;
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
	
	wls->SendSignal("GUI Window AddWidget",&contag);
}

GUIRoster::~GUIRoster ()
{
	EXP_SIGUNHOOK("Config XML Change /main/window", &GUIRoster::ReadConfig, 500);
	
	SaveConfig(false);
}

void
GUIRoster::ToggleButton(GtkWidget *widget, GUIRoster *c)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
		c->bstate[widget] = TRUE;
	else 
		c->bstate[widget] = FALSE;
		
	WokXMLTag empty(NULL, "empty");
	c->wls->SendSignal("Jabber Roster Recheck", empty);
}

int
GUIRoster::ReadConfig (WokXMLTag *tag)
{
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
	
	return true;
}

void
GUIRoster::SaveConfig(bool connect_sig)
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
	conftag.AddAttr("path", "/main/window");
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
				if( bstate[togg2] )
					(*riter)->AddAttr("view", "true");
			}
			else if ( (*riter)->GetFirstTag("show").GetBody() == "dnd")
			{
				if( bstate[togg3] )
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
	std::string datadir = PACKAGE_DATA_DIR;
	
	if(tag->GetFirstTag("item").GetTagList("resource").begin() == tag->GetFirstTag("item").GetTagList("resource").end())
		wls->SendSignal("Jabber Roster GetResource",tag);
	
	std::list <WokXMLTag*>::iterator tagiter;
	for( tagiter = tag->GetTagList("item").begin() ; tagiter != tag->GetTagList("item").end() ; tagiter++)
	{
		std::list <WokXMLTag*>::iterator riter;
		for( riter = (*tagiter)->GetTagList("resource").begin() ; riter != (*tagiter)->GetTagList("resource").end() ; riter++)
		{
			if( (*tagiter)->GetAttr("type") == "unavailable")
				(*riter)->AddAttr("icon", datadir + "/wokjab/offline.png");
			else if ( (*riter)->GetFirstTag("show").GetBody() != "")
				(*riter)->AddAttr("icon", icons[(*riter)->GetFirstTag("show").GetBody()]);
			else
				(*riter)->AddAttr("icon", datadir + "/wokjab/online.png");
		}
		if( (*tagiter)->GetTagList("resource").begin() == (*tagiter)->GetTagList("resource").end() )
		{
			(*tagiter)->AddAttr("icon", datadir + "/wokjab/offline.png");
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
	msgtag.AddText(xml_data);
	msgtag.AddAttr("session", tag->GetAttr("session"));
	wls->SendSignal("Jabber XML Send", &msgtag);
	
	return true;
}




