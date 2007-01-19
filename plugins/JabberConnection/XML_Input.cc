/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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


#include "XML_Input.h"

#define BUFFSIZE 8000

#include "staticfunctions.h"
#include "IQauth.h"

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>
#include <iostream>
#include <sstream>

#define NOTSEND current_tag=="presence" || current_tag == "message" || current_tag == "iq"

XML_Input::XML_Input (Connection *conn, WLSignal *wls, std::string session):
session(session),
conn(conn),
wls(wls)
{
	p = XML_ParserCreate (NULL);

	XML_SetUserData (p, this);
	XML_SetElementHandler (p, xml_start, xml_end);
	XML_SetCharacterDataHandler (p, xml_charhndl);
	ssl = NULL;
	depth = 0;
}

XML_Input::~XML_Input()
{
	XML_ParserFree(p);

}

int
XML_Input::get_depth ()
{
	return depth;
}

int
XML_Input::read_data (int source)
{
	char buffer[BUFFSIZE+1];
	int len;

	if(ssl)
	{
		len = SSL_read(ssl, buffer, BUFFSIZE);
		if ( len < 0 )
		{
			switch ( SSL_get_error(ssl, len) )
			{
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
				case SSL_ERROR_WANT_CONNECT:
				case SSL_ERROR_WANT_ACCEPT:
					return 2;
			}
		}
	}
	else
	{
		len = recv (source, buffer, BUFFSIZE, 0);
	}

	if (len == 0 || len == -1)  // wounder what this really could result in
	{/* Connection closed */
        std::stringstream sstr;
        sstr << len << " ssl: " << ssl;

        woklib_message(wls,"Assuming that the connection is closed..." + sstr.str());
		return 0;
	}

	buffer[len] = 0;

#ifdef DEBUG
	{
		WokXMLTag sigtag(NULL, "message");
		sigtag.AddAttr("type", "in");
		sigtag.AddTag("body").AddText(buffer);
		wls->SendSignal("Display Socket", sigtag);
	}
#endif // DEBUG
	WokXMLTag activetag (NULL, "active");
	activetag.AddAttr("session", session);
	wls->SendSignal("Jabber Session IO Active", activetag);
	wls->SendSignal("Jabber Session IO Active " + session, activetag);
	
	if (!XML_Parse (p, buffer, len, 0))
	{
		std::string msg;
		msg = "Parse error at line " ;
		msg += XML_GetCurrentLineNumber (p);
		msg += ":\n";
		msg +=  XML_ErrorString (XML_GetErrorCode (p));
		msg += '\n';

		WokXMLTag sigtag(NULL, "message");
		sigtag.AddTag("body").AddText(std::string("Error Socket Recv: ") + msg);
		wls->SendSignal("Display Error", sigtag);
		return 0;
	}

	if( len == BUFFSIZE )
		read_data(source);
	return 2;
}

void
XML_Input::start (const char *el, const char **attr)
{
	depth++;

	if(depth > 1 )
	{
		if(depth == 2)
			current_xml_tag = new WokXMLTag(NULL, el);
		else if (depth > 2)
			current_xml_tag = &current_xml_tag->AddTag(el);
		for (int i = 0; attr[i]; i += 2)
			current_xml_tag->AddAttr(attr[i], attr[i+1]);
	}

	if (depth == 2)
	{
		current_tag = el;
	}
	else if (depth == 1)
	{
		// This might not be the greatest place to put this...
		if (std::string(el) == "stream:stream")
		{
			std::string con_id;
			std::string strversion;
			float version;
			for (int i = 0; attr[i]; i += 2)
			{
				if( strcmp(attr[i], "id") == 0)
					con_id = attr[i+1];
				if( strcmp(attr[i], "version") == 0)
				{
					version = atof(attr[i+1]);
					strversion = attr[i+1];
				}
			}
			if ( con_id.size() )
				conn->SetConID(con_id);
			// This means we are connected
			WokXMLTag msg(NULL, "message");
			msg.AddAttr("session", session);
			if( version < 1 )
			{
				wls->SendSignal("Jabber Connection Established", &msg);
				new IQauth(wls, session); // Has selfdestruct...
			}
			return;
		}
	}

}

void
XML_Input::contence (const char *string, int len)
{
	if( current_xml_tag )
	{
		std::string str = std::string(string, 0 , len );

		current_xml_tag->AddText(str);
	}
}

void
XML_Input::end (const char *el)
{
	depth--;
	if (depth == 0)
	{

		// This might not be the greatest place to put this...
		if (std::string(el) == "stream:stream")  // Connection closing
		{
			WokXMLTag msg(NULL, "message");
			msg.AddAttr("session", session);
			wls->SendSignal("Jabber Connection Lost", &msg);
			return;
		}
	}
	else if ( depth == 1)
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddTag(current_xml_tag);
		msgtag.AddAttr("session", session);

		wls->SendSignal("Jabber XML Object " + current_tag, &msgtag);
		delete current_xml_tag;
		current_xml_tag = NULL;
	}
	else if ( depth > 1 )
		current_xml_tag = static_cast<WokXMLTag *>(current_xml_tag->GetParant());
}

void
XML_Input::SetSSL(::SSL * s)
{
	if ( ssl != s )
	{
		//XML_ParserFree(p);
		p = XML_ParserCreate (NULL); // FIX shouldnt this be cleared ?

		XML_SetUserData (p, this);
		XML_SetElementHandler (p, xml_start, xml_end);
		XML_SetCharacterDataHandler (p, xml_charhndl);
		depth = 0;
	}
	ssl = s;
}
