/***************************************************************************
 *  Copyright (C) 2003-2004  Kent Gustavsson <oden@gmx.net>
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


#ifndef __WOKJAB_XML_INPUT_H
#define __WOKJAB_XML_INPUT_H


//#include <glib.h>
#include <expat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>

class XML_Input;


#include <openssl/ssl.h>
#include "Connection.h"
#include "SSL.h"
#include <string>

using namespace Woklib;

class XML_Input
{
      public:
	XML_Input (Connection * conn, WLSignal *wls, std::string session);
	~XML_Input();
	
	int read_data (int source);
	int get_depth ();
	void SetSSL( ::SSL *s);
	// Don't get any ideas about useing these functions
	// Probably should have been using friends on them
	// but have forgotten how that works
	void start(const char *el, const char **attr);
	void contence(const char *string, int len);
	void end(const char *el);
	
	private:

	std::string session;
	std::string current_tag;
	WLSignal *wls;
	XML_Parser p;
	int depth;
	int socket;
	WokXMLTag *current_xml_tag;
	Connection *conn;
	::SSL *ssl;
};

#endif // __WOKJAB_XML_INPUT_H
