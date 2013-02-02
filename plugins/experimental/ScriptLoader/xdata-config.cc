/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2009 <nedo80@gmail.com>
 * 
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "xdata-config.h"

#include <sstream>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif



#include "../../../gettext.h"
#define _(string) gettext (string)

/** 
 * xdataConfig konstruktor hooks the initial signals
 */
xdataConfig::xdataConfig(WLSignal *wls) : WoklibPlugin(wls)
{
	id = 0;
	EXP_SIGHOOK("Jabber Disco Items Get Node http://jabber.org/protocol/commands", &xdataConfig::Commands, 500);
	EXP_SIGHOOK("Jabber AdHoc Command Exec configure", &xdataConfig::configure_request, 500);

	config = new WokXMLTag("config");
	EXP_SIGHOOK("Config XML Change /xdataconfig", &xdataConfig::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/xdataconfig");
	wls->SendSignal("Config XML Trigger", &conftag);


}

xdataConfig::~xdataConfig()
{
	std::map <std::string, xdataConfigSession *>::iterator iter;

	for( iter = sessions.begin() ; iter != sessions.end() ; iter++)
		delete iter->second;
}

/**
 * Checks if the user has the required permissions, probably should add a possiblity to use p2p or e2e signing support here to... 
 * tho from what I remember the p2p in jabber isn't suposed to be signing the messages for some odd reason...
 *
 * @param session holds the session the user is in.
 * @param jid holds the jid of the user in question
 * @return true or false depending if the user should have the rights to access the config or not.
 */
bool
xdataConfig::HasPermission(std::string session, std::string jid)
{
	if ( jid.find("/") != std::string::npos )
		jid = jid.substr(0, jid.find("/"));

	std::list <WokXMLTag *>::iterator jidlist;
	
	for( 	jidlist = config->GetFirstTag("permission").GetFirstTag("list").GetTagList("item").begin();
			jidlist != config->GetFirstTag("permission").GetFirstTag("list").GetTagList("item").end();
			jidlist++)
	{
		if ( (*jidlist)->GetAttr("data") == jid )
			return true;
	}
	
	return false;
}

/**
 * Reading the config
 */
int
xdataConfig::ReadConfig(WokXMLTag *tag)
{
	tag->GetFirstTag("config").GetFirstTag("permission").AddAttr("label", _("Allow Remote Configuration from"));
	tag->GetFirstTag("config").GetFirstTag("permission").AddAttr("type", "jidlist");
	
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));

	return 1;
}

/**
 * Function to handle the incomming command
 */
int
xdataConfig::configure_request(WokXMLTag *tag)
{	
	std::string tid = tag->GetFirstTag("iq").GetFirstTag("command","http://jabber.org/protocol/commands").GetAttr("sessionid");
	if ( sessions.find(tid) != sessions.end() )
	{
		sessions[tid]->Execute(tag);
	}
	else
	{
		std::stringstream sstr;
		sstr << id++ << "-";

		std::string letters = "abcdefghijklmnopqrstuvxyz1234567890";
		for(int i = 0; i < 10 ; i++)
		{
			sstr << letters[rand()%letters.length()];
		}

		sessions[sstr.str()] = new xdataConfigSession(wls, *tag, sstr.str());
	}
	
	return 1;
}

int 
xdataConfig::Commands(WokXMLTag *tag)
{

	if ( ! HasPermission(tag->GetAttr("session"), tag->GetAttr("jid")))
		return 1;
		
	
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", tag->GetAttr("session"));
	wls->SendSignal("Jabber Connection GetUserData", &querytag);
		
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("node", "configure");
	item.AddAttr("name", "Configure");
	item.AddAttr("jid", querytag.GetFirstTag("item").GetFirstTag("jid").GetBody());

	return 1;
}