/***************************************************************************
 *  Copyright (C) 2003-2009  Kent Gustavsson <oden@gmx.net>
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


#include "IQHandler.h"
#include <iostream>
#include <sstream>

using std::string;
using std::cerr;
using std::endl;

IQHandler::IQHandler (WLSignal *wls):
WLSignalInstance ( wls )
{
	EXP_SIGHOOK("Jabber XML Object iq", &IQHandler::xml, 1000);
	EXP_SIGHOOK("Jabber XML IQ Send", &IQHandler::sendxmliqsig, 1000);
	
	number = 0;
}

IQHandler::~IQHandler ()
{
}

int
IQHandler::sendxmliqsig( WokXMLTag *tag) 
{
	std::string iqhook;
	
	std::stringstream str;
	str << "wokjab" << number++;
	iqhook = str.str();
	
	tag->GetFirstTag("iq").AddAttr("id",  iqhook );
	wls->SendSignal("Jabber XML Send" , tag);
	
	return (1);
}

int
IQHandler::xml (WokXMLTag *xml)
{
	std::string type = xml->GetFirstTag("iq").GetAttr("type");
	std::string id = xml->GetFirstTag("iq").GetAttr("id");
	std::string to = xml->GetFirstTag("iq").GetAttr("to");
	std::string from = xml->GetFirstTag("iq").GetAttr("from");

	
	if(type == "result" || type == "error")
	{
		wls->SendSignal(std::string(std::string("Jabber XML IQ ID ") + id).c_str(), xml);
	}
	else if( type == "get" || type == "set" )
	{
		WokXMLTag tmptag(*xml);
		std::list<WokXMLTag *> &tags = tmptag.GetFirstTag("iq").GetTags();
		std::list<WokXMLTag *>::iterator iter;
		
		for( iter = tags.begin() ; iter != tags.end() ; iter++)
		{
			std::string xmlns;
			if((*iter)->GetAttr("xmlns").length())
				xmlns = " xmlns:" + (*iter)->GetAttr("xmlns");
			
			if(!wls->SendSignal("Jabber XML IQ New "+(*iter)->GetName()+" " + type + xmlns, xml))
			{
				WokXMLTag reptag(*xml);
				WokXMLTag errortag = reptag.GetFirstTag("iq");
				WokXMLTag &er = errortag.AddTag("error");
				errortag.AddAttr("type", "error");
				errortag.AddAttr("to", errortag.GetAttr("from"));
				errortag.RemoveAttr("from");
				
				er.AddAttr("type", "cancel");
				er.AddTag("service-unavailable").AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
				
				wls->SendSignal("Jabber Send XML" , &reptag);
			}
		}
	}
	return 1;
}
