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

#ifndef __WOKJAB_XML_OUTPUT_H
#define __WOKJAB_XML_OUTPUT_H

class XML_Output;

#include <string>
#include <openssl/ssl.h>
#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class XML_Output : public WLSignalInstance
{
	public:
	XML_Output(WLSignal *wls, std::string session);
	~XML_Output();

	void set_socket(int socket_nr);
	void SetSSL( ::SSL *s);


	int sendxml(std::string data);
	int sendxml(const char *data);
	int SocketAvailibule(WokXMLTag *tag);
	void code_msg(std::string data);
	int SignalDisconnect(WokXMLTag *tag);
	
	private:
		std::string buffer;
		std::string signal_out;
		std::string session;
		bool transmitting;
		int socket_nr;
		::SSL *ssl;
};





#endif // __WOKJAB_XML_OUTPUT_H
