/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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

#ifndef __JEP_65_SESSION_H
#define __JEP_65_SESSION_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>

#include <gtk/gtk.h>
#include "map"
#include "fstream"

using namespace Woklib;

class jep65Session : public WLSignalInstance
{
	public:
		jep65Session(WLSignal *wls, WokXMLTag *xml);
		~jep65Session();

	
		int GetSocket();
		int ReadData(WokXMLTag *xml);
		int SendInitData();
		int OpenConnection(std::string host, int port);
		int senddata(char *data, uint len);
	protected:
		WokXMLTag *orig;
		std::string initiator;
		std::string target;
		std::string sid;
		std::string lsid;
		std::string sha1;
		std::string filename;
		std::string session;
		std::ofstream file;
		int socket_nr;
		unsigned long long pos;
		int stage;
};

#endif //__JEP_65_SESSION_H
