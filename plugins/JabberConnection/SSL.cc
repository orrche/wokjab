/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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



/* Side note 
 * This file has been recovered from a deletion on a reiserfs partition by using 
 * grep on the partition /dev/hda1 ;) 
 */

#include "SSL.h"
#include "IQauth.h"

namespace Woklib
{
	SSL::SSL(WLSignal *wls, Connection *conn) : WLSignalInstance(wls),
	conn(conn)
	{
		SSL_library_init();
		
		// Kinda wrong no ?
		EXP_SIGHOOK("Jabber XML Object stream:features", &SSL::StartSession, 500);
		EXP_SIGHOOK("Jabber XML Object proceed", &SSL::Proceed, 500);
		
		initiated = false;
	}
	
	
	SSL::~SSL()
	{
		
	}
	
	int
	SSL::StartSession(WokXMLTag *tag)
	{
		if( tag->GetAttr("session") == conn->session)
		{
			if ( !initiated && tag->GetFirstTag("stream:features").GetTagList("starttls").size() )
			{
				WokXMLTag msgtag (NULL, "message");
				msgtag.AddAttr("session", tag->GetAttr("session"));
				WokXMLTag &tlstag = msgtag.AddTag("starttls");
				tlstag.AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");
				
				wls->SendSignal("Jabber XML Send", &msgtag);
			}
			else
			{
				WokXMLTag msg(NULL, "message");
				msg.AddAttr("session", tag->GetAttr("session"));
				wls->SendSignal("Jabber Connection Established", &msg);
				new IQauth(wls, conn->GetConID(), tag->GetAttr("session"));
			}
		}
	
	
		return true;
	}
	
	int
	SSL::Proceed(WokXMLTag *tag)
	{
		if ( tag->GetAttr("session") != conn->session)
			return true;
		SSL_METHOD *meth;
		SSL_CTX *ctx;
		::SSL *ssl;
		BIO *sbio;
						
		meth=SSLv23_method();
		ctx=SSL_CTX_new(meth);
		sslsession[tag->GetAttr("session")] = ctx;
			
		ssl=SSL_new(ctx);
		WokXMLTag socktag(NULL, "socket");
		socktag.AddAttr("session", tag->GetAttr("session"));
		wls->SendSignal("Jabber Connection GetSocket", &socktag);
		sbio=BIO_new_socket(atoi(socktag.GetAttr("socket").c_str()),BIO_NOCLOSE);
		SSL_set_bio(ssl,sbio,sbio);
	
		/* Wow really super code here hehe */
		if(SSL_connect(ssl)<=0)
		 std::cout<< "Didn't go so good.. :/" << std::endl;
		else
		{
			conn->SetSSL(ssl);
			conn->sendinit();
		}
		initiated = true;
		return true;
	}

};
