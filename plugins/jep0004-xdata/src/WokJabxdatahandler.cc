/***************************************************************************
+- *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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
#include "../include/WokJabxdatahandler.h"
#include "../include/WokJabxdataWidget.h"

WokJabxdatahandler::WokJabxdatahandler(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber jabber:x:data Init", &WokJabxdatahandler::init, 1000);
	id = 0;
}

WokJabxdatahandler::~WokJabxdatahandler()
{	

}

void 
WokJabxdatahandler::init(WokXMLTag *tag)
{
	char buf[20];
	
	if ( tag->GetAttr("signal").empty() )
	{
		sprintf(buf, "%d", id++);
		std::string signal("Jabber jabber:x:data ID ");
		signal += buf;
		tag->AddAttr("signal", signal);
	}
	tag->AddAttr("id", buf);
	new WokJabxdataWidget(wls, tag);	
}
