/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: SSL
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Aug 26 00:13:27 2005
//

#ifndef _WLSSL_H_
#define _WLSSL_H_

namespace Woklib
{
	class SSL;
}

#include "Connection.h"

#include <openssl/ssl.h>
namespace Woklib
{
	class SSL : public WLSignalInstance
	{
		public:
			SSL(WLSignal *wls, Connection *conn);
			 ~SSL();
		
			
			int StartSession(WokXMLTag *tag);
			int Proceed(WokXMLTag *tag);
			int SocketAvailibule(WokXMLTag *tag);
			
			::SSL *ssl;
		protected:
			std::map <std::string, SSL_CTX *> sslsession;
			Connection *conn;
			bool initiated;
			std::string signal_out;
	};
}

#endif	//_WLSSL_H_

