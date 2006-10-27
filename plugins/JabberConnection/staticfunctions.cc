/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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


#include "staticfunctions.h"
#include "XML_Input.h"

#include <stdio.h>

void
xml_start (void *data, const char *el, const char **attr)
{
	XML_Input *in;
	in = (XML_Input *) data;

	in->start(el,attr);

	return;
}

void
xml_end (void *data, const char *el)
{
	XML_Input *in;
	in = (XML_Input *) data;

	in->end(el);

	return;
}

void
xml_charhndl (void *data, const char *string, int len)
{
	XML_Input *in;
	in = (XML_Input *) data;

	in->contence(string,len);

	return;
}
