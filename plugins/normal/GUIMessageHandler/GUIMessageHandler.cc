/***************************************************************************
 *  Copyright (C) 2003-2010  Kent Gustavsson <nedo80@gmail.com>
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

#include <fstream>
#include "GUIMessageHandler.h"
#include "GUIMessageWidget.h"

using std::string;

#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __WIN32
#include <io.h>
#endif

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "../../../gettext.h"
#define _(string) gettext (string)


GUIMessageHandler::GUIMessageHandler(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML Message Normal", &GUIMessageHandler::new_message, 999);
	EXP_SIGHOOK("Jabber XML Message Normal", &GUIMessageHandler::CopyBody, 0);

	EXP_SIGHOOK("Jabber GUI MessageDialog Open", &GUIMessageHandler::OpenDialog, 1000);
	EXP_SIGHOOK("Jabber XML Presence", &GUIMessageHandler::Presence, 1000);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &GUIMessageHandler::JIDMenu, 1000);
	EXP_SIGHOOK("Jabber GUI MessageDialogOpenMenu", &GUIMessageHandler::MenuOpenDialog, 1000);

	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /chat/window", &GUIMessageHandler::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/chat/window");
	wls->SendSignal("Config XML Trigger", &conftag);

	std::string datadir = PACKAGE_DATA_DIR;
	pix_msg = gdk_pixbuf_new_from_file(std::string(datadir + "/wokjab/msg.png").c_str(),NULL);
	msgicon = datadir + "/wokjab/msg.png";

	id = 0;
}


GUIMessageHandler::~GUIMessageHandler()
{
}

int
GUIMessageHandler::Config(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));


	// Caching the lock on resource value
	if ( config->GetFirstTag("disable_chat_window_lock").GetAttr("data") == "")
	{
		tag->GetFirstTag("config").GetFirstTag("disable_chat_window_lock").AddAttr("show", "true");
		tag->GetFirstTag("config").GetFirstTag("disable_chat_window_lock").AddAttr("type", "bool");
		tag->GetFirstTag("config").GetFirstTag("disable_chat_window_lock").AddAttr("label", _("Disabel resource lock"));
		tag->GetFirstTag("config").GetFirstTag("disable_chat_window_lock").AddAttr("tooltip", _("Disables locking of resource in chat window."));
	}
	if ( config->GetFirstTag("disable_chat_window_lock").GetAttr("data") != "false")
	{
		lock_on_resource = false;
	}
	else
	{
		lock_on_resource = true;
	}


	return 1;
}

void
GUIMessageHandler::TriggerEvent(WokXMLTag *tag)
{
	// Getting the Nickname, and setting the jid variable to the jid without resource
	std::string nick;
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &nick_itemtag = querytag.AddTag("item");
	string jid = tag->GetFirstTag("message").GetAttr("from");
	string::size_type pos = jid.find("/");
  if( pos != std::string::npos )
          jid = jid.substr(0, pos);
	nick_itemtag.AddAttr("session", tag->GetAttr("session"));
	nick_itemtag.AddAttr("jid", jid);
	wls->SendSignal("Jabber Roster GetResource", &querytag);
	if ( querytag.GetFirstTag("item").GetAttr("nick") != "" )
		nick = querytag.GetFirstTag("item").GetAttr("nick");
	else
		nick = jid;
	
	// Creating the event
	WokXMLTag eventtag(NULL, "event");
	eventtag.AddAttr("type", "message");
	WokXMLTag &itemtag = eventtag.AddTag("item");
	itemtag.AddAttr("id", "Message " + tag->GetAttr("session") + " " + tag->GetFirstTag("message").GetAttr("from"));
	itemtag.AddAttr("icon", msgicon);
	itemtag.AddAttr("jid", tag->GetFirstTag("message").GetAttr("from"));
	itemtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &desc = itemtag.AddTag("description");
	desc.AddText(nick);
	desc.AddText("\n\t");
	desc.AddText(tag->GetFirstTag("message").GetFirstTag("body").GetBody());

	WokXMLTag &command = itemtag.AddTag("commands").AddTag("command");
	command.AddAttr("name", "Open Dialog");
	WokXMLTag &sig = command.AddTag("signal");
	sig.AddAttr("name", "Jabber GUI MessageDialog Open");
	WokXMLTag &item = sig.AddTag("item");
	if ( lock_on_resource )
		item.AddAttr("jid", tag->GetFirstTag("message").GetAttr("from"));
	else
		item.AddAttr("jid", jid);
	item.AddAttr("session", tag->GetAttr("session"));
	
	wls->SendSignal("Jabber Event Add", &eventtag);
}

int
GUIMessageHandler::CopyBody(WokXMLTag *tag)
{
	tag->AddTag(&tag->GetFirstTag("message").GetFirstTag("body"));

	return 1;
}

int
GUIMessageHandler::new_message(WokXMLTag *tag)
{
	if(tag->GetAttr("displayed") == "true")
		return true;

	std::string jid;
	std::string session;
	std::string resource;

	WokXMLTag *message_tag;
	message_tag = &tag->GetFirstTag("message");

	
	if( message_tag->GetFirstTag("body").GetBody() == "" && tag->GetTagList("command").empty() )
		return 1;

	if ( message_tag->GetAttr("type") == "error" )
	{
		tag->GetFirstTag("body").RemoveBody();
		WokXMLTag &span = tag->GetFirstTag("body").AddTag("span");
		span.AddAttr("style", "color:#FF0000;");
		span.AddText("Failed to send this message:\n" + message_tag->GetFirstTag("body").GetBody());
		
	}
	
	string::size_type pos = message_tag->GetAttr("from").find("/");
	if( pos != std::string::npos )
	{
		jid = message_tag->GetAttr("from").substr(0, pos);
		resource = message_tag->GetAttr("from").substr(pos+1);
	}
	else
		jid = message_tag->GetAttr("from");

	session = tag->GetAttr("session");


	wls->SendSignal("Jabber XML Message From " + session + " " + message_tag->GetAttr("from"), tag);
	wls->SendSignal("Jabber XML Message From " + session + " " + jid, tag);

	if( tag->GetAttr("displayed") != "true")
	{
		TriggerEvent(tag);
	}

	return 1;
}

int
GUIMessageHandler::JIDMenu( WokXMLTag *tag)
{
	WokXMLTag *item;
	item = & tag->AddTag("item");
	item->AddAttr("name", "Open Dialog");
	item->AddAttr("signal", "Jabber GUI MessageDialogOpenMenu");

	return 1;
}

int
GUIMessageHandler::MenuOpenDialog( WokXMLTag *tag)
{
	OpenDialog(tag); // Was the same
	return 1;
}

int
GUIMessageHandler::OpenDialog(WokXMLTag *tag)
{
	std::string jid;
	std::string session;

	jid = tag->GetAttr("jid");
	session = tag->GetAttr("session");

	WokXMLTag emptytag(NULL, "activate");
	if (!wls->SendSignal("Jabber GUI Message Activate " + session + " " + jid, &emptytag ))
	{
		string nick;
		new GUIMessageWidget(wls, session, tag->GetAttr("jid"), id++);
		
		
		WokXMLTag eventtag(NULL, "event");
		eventtag.AddAttr("type", "message");
		WokXMLTag &itemtag = eventtag.AddTag("item");
		itemtag.AddAttr("id", "Message " + session + " " + tag->GetAttr("jid"));
		
		wls->SendSignal("Jabber Event Remove", &eventtag);
		
	}
	return 1;

}

int
GUIMessageHandler::Presence(WokXMLTag *tag)
{
	WokXMLTag &tag_presence = tag->GetFirstTag("presence");
	std::string jid;

	jid = tag_presence.GetAttr("from").substr(0, tag_presence.GetAttr("from").find("/"));

	wls->SendSignal(std::string("Jabber XML Presence From ") + tag->GetAttr("session") + " " + tag_presence.GetAttr("from"), tag);
	wls->SendSignal(std::string("Jabber XML Presence From ") + tag->GetAttr("session") + " " + jid, tag );

	return 1;
}
