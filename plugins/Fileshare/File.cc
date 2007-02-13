/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "File.h"

File::File(WLSignal *wls, WokXMLTag &msg) : WLSignalInstance(wls),
xml(msg)
{
	EXP_SIGHOOK("Jabber XML IQ ID " + msg.GetFirstTag("iq").GetAttr("id"), &File::FileResponse, 1000);
	/*

	*/
}

File::~File()
{


}

int 
File::FileResponse(WokXMLTag *tag)
{
	std::cout << "XML: " << *tag << std::endl;
	sid = tag->GetFirstTag("iq").GetFirstTag("fileshare").GetAttr("sid");
	std::cout << "Sid: " << sid << std::endl;
	
	EXP_SIGHOOK("Jabber Stream File Incomming " + sid, &File::Incomming, 100);
	EXP_SIGHOOK("Jabber Stream RequestAuthorisation", &File::Auth, 1000);
	EXP_SIGHOOK("Jabber Stream File Status Finished", &File::Finished, 1000);
	return 1;
}

int
File::Auth(WokXMLTag *tag)
{
	if ( tag->GetAttr("sid") == sid )
	{
		lsid = "local:" + sid;
		WokXMLTag &file = tag->AddTag("file");
		file.AddAttr("lsid", lsid);
		file.AddAttr("name", "/tmp/filelist.xml.gz");	
	}
	return 1;
}

int
File::Finished(WokXMLTag *tag)
{

	return 1;
}

int
File::Incomming(WokXMLTag *tag)
{
	std::cout << "What the fuck.. " <<std::endl;
	
	WokXMLTag msg(NULL, "message");
	msg.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iq = msg.AddTag("iq");
	iq.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
	iq.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
	iq.AddAttr("type", "result");
	WokXMLTag &si = iq.AddTag("si");
	si.AddAttr("xmlns", "http://jabber.org/protocol/si");
	WokXMLTag &feature = si.AddTag("feature");
	feature.AddAttr("xmlns", "http://jabber.org/protocol/feature-neg");
	WokXMLTag &x = feature.AddTag("x");
	x.AddAttr("type", "submit");
	x.AddAttr("xmlns", "jabber:x:data");
	WokXMLTag &field = x.AddTag("field");
	field.AddAttr("var", "stream-method");
	WokXMLTag &value = field.AddTag("value");
	value.AddText("http://jabber.org/protocol/bytestreams");
	
	wls->SendSignal("Jabber XML Send", msg);
	std::cout << "hum" << std::endl;

	return 1;
}
