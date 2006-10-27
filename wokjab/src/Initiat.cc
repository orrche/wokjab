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


#include "../include/Initiat.h"
#include <string>
#include <sys/utsname.h>
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

Initiat::Initiat (WLSignal *wls, WokLib *sj):WLSignalInstance (wls),
sj(sj)
{
	EXP_SIGHOOK("Woklib Socket In Add", &Initiat::AddListener, 800);
	EXP_SIGHOOK("Woklib Socket Out Add", &Initiat::AddWatcher, 800);
	EXP_SIGHOOK("Jabber Connection Authenticated", &Initiat::Connected, 200);
	EXP_SIGHOOK("Config XML Change /", &Initiat::Plugins, 200);
	EXP_SIGHOOK("Jabber XML IQ New query get xmlns:jabber:iq:version", &Initiat::Version, 500);
	logged_in = false;


	/*
	WokXMLTag conf2tag(NULL, "config");
	conf2tag.AddAttr("path", "/plugin");
	wls->SendSignal("Config XML Trigger", &conf2tag);

	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/");
	wls->SendSignal("Config XML Trigger", &conftag);
	*/
}


Initiat::~Initiat ()
{
}

int
Initiat::Plugins(WokXMLTag *xml)
{
	std::cout << "PLUGINS: " << *xml << std::endl;
	
	return 1;
}

int
Initiat::AddWatcher(WokXMLTag *xml)
{
	static int id = 0;
	int socket = atoi(xml->GetAttr("socket").c_str());
	char freebuf[20];
	id++;
	sprintf(freebuf, "Socket Availibule %d", id);
	freesockets[atoi(xml->GetAttr("socket").c_str())] = freebuf;
	xml->AddAttr("signal", freebuf);
	
	GIOChannel *source = g_io_channel_unix_new (socket);
	errorsockets[socket].push_back( g_io_add_watch(source, (GIOCondition)(G_IO_OUT | G_IO_NVAL), (GIOFunc) &Initiat::input_callback, this));

	return 0;
}


int
Initiat::AddListener(WokXMLTag *xml)
{
	static int id = 0;
	char buf[20];
	int socket = atoi(xml->GetAttr("socket").c_str());
	id++;
	sprintf(buf, "Socket Activate %d", id);
	sockets[atoi(xml->GetAttr("socket").c_str())] = buf;
	xml->AddAttr("signal", buf);
	
	GIOChannel *source = g_io_channel_unix_new (socket);
	errorsockets[socket].push_back( g_io_add_watch(source, (GIOCondition)(G_IO_IN | G_IO_NVAL), (GIOFunc) &Initiat::input_callback, this));
	
	return 0;
}

int
Initiat::AddErrorsock(GIOChannel *source, int socket)
{
	errorsockets[socket].push_back(g_io_add_watch( source, G_IO_NVAL, (GIOFunc) &Initiat::input_callback, this));
}

gboolean
Initiat::input_callback(GIOChannel *source, GIOCondition condition, Initiat *c)
{
	int socket = g_io_channel_unix_get_fd (source);
	int ret = 1;
	WokXMLTag sigdata(NULL, "socket");
	if( condition & G_IO_IN )
	{
		ret = c->wls->SendSignal(c->sockets[socket], sigdata);
	}
	if( condition & G_IO_OUT )
	{
		ret = c->wls->SendSignal(c->freesockets[socket], sigdata);
	}
	if( condition & G_IO_NVAL )
	{
		std::vector<uint>::iterator iter;
		
		for( iter = c->errorsockets[socket].begin() ; iter != c->errorsockets[socket].end() ; iter++)
			g_source_remove(*iter);
		return FALSE;
	}
	
	if ( ret == 0 ) 
	{
		g_io_channel_close (source);
		return FALSE;
	}
	
	if(sigdata.GetAttr("error").size() || sigdata.GetAttr("stop").size())
	{
		if ( condition == G_IO_IN )
		{
			c->sockets.erase(socket);
		}
		else if ( condition == G_IO_OUT )
		{
			c->freesockets.erase(socket);
		}

		return FALSE;
	}
	return TRUE;
}

int 
Initiat::Connected(WokXMLTag *tag) 
{
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	msgtag.AddTag("presence").AddTag("priority").AddText("50");
	wls->SendSignal("Jabber XML Presence Send", &msgtag);
	return 1;
};

int
Initiat::Version(WokXMLTag *tag)
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "result");
	iqtag.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
	iqtag.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "jabber:iq:version");
	querytag.AddTag("name").AddText("WokJab (http://wokjab.sf.net/)");

#ifdef VERSION
	querytag.AddTag("version").AddText(VERSION);
#endif

	
	struct utsname buf;
	uname(&buf);
	int i(0);
	
	querytag.AddTag("os").AddText(std::string(buf.sysname) + " " + buf.release);
	wls->SendSignal("Jabber XML Send", &msgtag);
	
	return 1;
}
