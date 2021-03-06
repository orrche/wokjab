/**************************************************************************
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


#include "../include/Initiat.h"
#include <string>
#ifdef __WIN32
#include <winsock.h>
typedef unsigned int uint;
#else
#include <sys/utsname.h>
#endif

#include <sstream>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

Initiat::Initiat (WLSignal *wls, WokLib *sj, int argc, char **argv):WLSignalInstance (wls),
sj(sj)
{
	args = new WokXMLTag (NULL, "args");
	std::string singlearg;
	for( int i = 0; i < argc ; i++)
	{
		std::string arg = argv[i];
		if ( !arg.empty() )
		{
			if ( arg[0] == '-' )
			{
				if ( arg.size() > 1 && arg[1] == '-' )
				{
					WokXMLTag &argtag = args->GetFirstTag("fullsize").AddTag("arg");
					argtag.AddAttr("name", arg.substr(2));
				}
				else
				{
					for( int n = 0 ; n < arg.substr(1).size() ; n++ )
					{
						if ( singlearg.find(arg[1+n]) == std::string::npos)
							singlearg += arg[1+n];
					}
				}
			}
		}
	}
	args->GetFirstTag("singlearg").AddAttr("data", singlearg);
	
	EXP_SIGHOOK("Woklib Socket In Add", &Initiat::AddListener, 800);
	EXP_SIGHOOK("Woklib Socket Out Add", &Initiat::AddWatcher, 800);
	EXP_SIGHOOK("Wokjab GetArgs", &Initiat::GetArgs, 500);

	EXP_SIGHOOK("Config XML Change /", &Initiat::Plugins, 200);
	EXP_SIGHOOK("Jabber XML IQ New query get xmlns:jabber:iq:version", &Initiat::Version, 500);
	EXP_SIGHOOK("Jabber Connection Authenticated", &Initiat::Connected, 200);
	
	logged_in = false;

#ifdef __WIN32
    WSAData wsad;
    if ( WSAStartup(1,&wsad) != 0)
    {
        woklib_error(wls,"Couldn't init WSA no connections will be possibule");
    }
#endif
	
	EXP_SIGHOOK("Jabber AutoConnect", &Initiat::AccountConfig, 500);
	EXP_SIGHOOK("Program Start", &Initiat::Ready, 500);
}


Initiat::~Initiat ()
{
	delete args;
#ifdef __WIN32
    WSACleanup();
#endif
}

int
Initiat::GetArgs(WokXMLTag *xml)
{
	xml->AddTag(args);
	return 1;
}

int
Initiat::Plugins(WokXMLTag *xml)
{
	return 1;
}

int
Initiat::Ready(WokXMLTag *tag)
{
	wls->SendSignal("Jabber AutoConnect", tag);
	return 1;	
}

int
Initiat::AccountConfig(WokXMLTag *unimportanttag)
{
	WokXMLTag tag("config");
	tag.AddAttr("path", "/connect/window");
	wls->SendSignal("Config XML GetConfig", tag);
	
	std::list <WokXMLTag *>::iterator iter;

	for( iter = tag.GetFirstTag("config").GetTagList("account").begin() ; iter != tag.GetFirstTag("config").GetTagList("account").end() ; iter++)
	{
		if ( (*iter)->GetFirstTag("no_auto").GetAttr("data") == "false" )
		{
			std::string nick,server,password,resource, port, prio;

			char buf[20];
			server = (**iter).GetFirstTag("server").GetAttr("data");
			nick = (**iter).GetFirstTag("nick").GetAttr("data");
			password = (**iter).GetFirstTag("password").GetAttr("data");
			resource = (**iter).GetFirstTag("resource").GetAttr("data");
			prio = (**iter).GetFirstTag("prio").GetAttr("data");
			port = (**iter).GetFirstTag("port").GetAttr("data");
				
			int at_position = 0;
			if ( nick.find("@") == std::string::npos )
				nick += "@" + server;
			
			at_position = nick.find("@");

			WokXMLTag consig(NULL, "connect");
			consig.AddAttr("host", server);
			consig.AddAttr("server", nick.substr(at_position+1, nick.size()-at_position-1));
			consig.AddAttr("username", nick.substr(0,at_position));
			consig.AddAttr("password", password);
			consig.AddAttr("resource", resource);
			consig.AddAttr("port", port);
			consig.AddAttr("prio", prio);
			consig.AddAttr("type", "1");
			wls->SendSignal ("Jabber Connection Connect", &consig);
		
			
		}
	}
	
	
	return 1;
}

int
Initiat::AddWatcher(WokXMLTag *xml)
{
	static int id = 0;
	int socket = atoi(xml->GetAttr("socket").c_str());
	char freebuf[40];
	id++;
	sprintf(freebuf, "Socket Availibule %d", id);

	freesockets[socket] = freebuf;
	xml->AddAttr("signal", freebuf);

	GIOChannel *source = g_io_channel_unix_new (socket);
	errorsockets[socket].push_back( g_io_add_watch(source, (GIOCondition)(G_IO_OUT | G_IO_NVAL | G_IO_ERR ), (GIOFunc) &Initiat::input_callback, this));

	return 0;
}


int
Initiat::AddListener(WokXMLTag *xml)
{
	static int id = 0;
	char buf[40];
	int socket = atoi(xml->GetAttr("socket").c_str());
	id++;
	sprintf(buf, "Socket Activate %d", id);
	sockets[atoi(xml->GetAttr("socket").c_str())] = buf;
	xml->AddAttr("signal", buf);

	GIOChannel *source = g_io_channel_unix_new (socket);
	errorsockets[socket].push_back( g_io_add_watch(source, (GIOCondition)(G_IO_IN | G_IO_NVAL | G_IO_ERR ), (GIOFunc) &Initiat::input_callback, this));

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
	if( condition & G_IO_NVAL || condition & G_IO_ERR )
	{
		std::vector<uint>::iterator iter;

		if ( c->freesockets.find(socket) != c->freesockets.end() )
		{
			sigdata.AddAttr("error", "closed");
			c->wls->SendSignal(c->freesockets[socket], sigdata);
			c->freesockets.erase(socket);
		}
		if ( c->sockets.find(socket) != c->sockets.end() )
		{
			sigdata.AddAttr("error", "closed");
			c->wls->SendSignal(c->sockets[socket], sigdata);
			c->sockets.erase(socket);
		}
		
		for( iter = c->errorsockets[socket].begin() ; iter != c->errorsockets[socket].end() ; iter++)
			g_source_remove(*iter);

		g_io_channel_unref (source);
		return FALSE;
	}

	if(sigdata.GetAttr("error").size() || sigdata.GetAttr("stop").size() || ret == 0)
	{
		if ( condition == G_IO_IN )
		{
			c->sockets.erase(socket);
		}
		else if ( condition == G_IO_OUT )
		{
			c->freesockets.erase(socket);
		}
		
		g_io_channel_unref (source);
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
	querytag.AddTag("name").AddText("WokJab (http://wokjab.nedo.se/)");

#ifdef VERSION
	querytag.AddTag("version").AddText(VERSION);
#endif


#ifdef __WIN32
#else
	struct utsname buf;
	uname(&buf);
	int i(0);

	querytag.AddTag("os").AddText(std::string(buf.sysname) + " " + buf.release);
#endif
	wls->SendSignal("Jabber XML Send", &msgtag);

	return 1;
}
