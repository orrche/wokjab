/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
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


//
// Class: User
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug 29 09:45:04 2006
//

#include "User.h"


User::User(WLSignal *wls, std::string jid) : WLSignalInstance(wls),
jid(jid)
{
	
}


User::~User()
{

}

void
User::SetHash(std::string hash)
{
	this->hash = hash;
}

const std::string &
User::GetHash()
{
	return hash;
}

void
User::SetID(std::string id)
{
	this->id = id;
}

const std::string &
User::GetID()
{
	return id;
}

