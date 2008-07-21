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


#include "include/Resource.h"

#include <iostream>
#include <sstream>

Resource::Resource()
{
	t = time(0);
}


Resource::~Resource()
{
	
}

const int &
Resource::GetPriority()
{
	return priority;
}

const std::string & 
Resource::GetShow()
{
	return show;
}

const std::string &
Resource::GetStatus()
{
	return status;
}

const std::string & 
Resource::GetResource()
{
	return resource;
}

void
Resource::SetPriority(const int & p)
{
	priority = p;
}

std::string
Resource::GetLogontime ()
{
	std::stringstream str;
	str << t;
	return str.str();	
}

void
Resource::SetShow( const std::string &show)
{
	this->show = show;
}

void
Resource::SetResource( const std::string &resource)
{
	this->resource = resource;
}

void
Resource::SetStatus( const std::string &status)
{
	this->status = status;
}
