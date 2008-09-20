/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
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

#include "jabber-reg-handler.hpp"

#ifndef _
#define _(x) x
#endif

/*
<signal level='0' name='Jabber RegisterJID'>
 <connect host='localhost' password='mat' port='5222' prio='50' resource='home' server='localhost' type='1' username='nedo_4'></connect>
</signal>
*/

JabberRegHandler::JabberRegHandler(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML Object stream:features", &JabberRegHandler::NewSession, 500);
	EXP_SIGHOOK("Jabber RegisterJID", &JabberRegHandler::Init, 500);
	EXP_SIGHOOK("Get Main Menu", &JabberRegHandler::Menu, 500);
	EXP_SIGHOOK("Jabber RegisterAccountRequestMenu", &JabberRegHandler::MenuAction, 500);
	EXP_SIGHOOK("Jabber RegisterAccount x:data resp", &JabberRegHandler::XDataResp, 0);

}

int 
JabberRegHandler::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Register Account Request");
	item.AddAttr("signal", "Jabber RegisterAccountRequestMenu");
	
	return 1;
}

int
JabberRegHandler::MenuAction(WokXMLTag *tag)
{
	/*
	<x xmlns='jabber:x:data'
	   type='{form-type}'>
	  <title/>
	  <instructions/>
	  <field var='field-name'
			 type='{field-type}'
			 label='description'>
		<desc/>
		<required/>
		<value>field-value</value>
		<option label='option-label'><value>option-value</value></option>
		<option label='option-label'><value>option-value</value></option>
	  </field>
	</x>
	*/
	
	WokXMLTag form("form");
	form.AddAttr("signal", "Jabber RegisterAccount x:data resp");
	WokXMLTag &x = form.AddTag("x", "jabber:x:data");
	WokXMLTag &field = x.AddTag("field");
	field.AddAttr("type", "text-single");
	field.AddAttr("var", "server");
	field.AddAttr("label", _("Server"));
	
	wls->SendSignal("Jabber jabber:x:data Init", form);
	return 1;
}

int
JabberRegHandler::XDataResp(WokXMLTag *tag)
{
	WokXMLTag reg("register");
	reg.AddAttr("host", tag->GetFirstTag("x", "jabber:x:data").GetFirstTag("field").GetFirstTag("value").GetBody());
	reg.AddAttr("port", "5222");
	reg.AddAttr("type", "1");
	reg.AddAttr("server", tag->GetFirstTag("x", "jabber:x:data").GetFirstTag("field").GetFirstTag("value").GetBody());
	
	wls->SendSignal("Jabber RegisterJID",reg);
	return 1;
}

int
JabberRegHandler::Init(WokXMLTag *tag)
{
	wls->SendSignal("Jabber Connection Connect",tag);
	sessions[tag->GetAttr("session")] = new Session(wls, *tag, this);
	
	return 1;
}

void
JabberRegHandler::Remove(std::string session)
{
	if ( sessions.find(session)  != sessions.end())
	{
		delete sessions[session];
		sessions.erase(session);
	}
}

int
JabberRegHandler::NewSession(WokXMLTag *tag)
{
	if( sessions.find(tag->GetAttr("session")) != sessions.end() )
	{
		
		sessions[tag->GetAttr("session")]->Connect(tag);
		
		return 0;
	}
	
	return 1;
}

