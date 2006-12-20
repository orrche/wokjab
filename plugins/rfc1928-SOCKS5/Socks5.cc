/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Socks5
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Dec 19 21:47:49 2006
//

#include "Socks5.h"
#include "Socks5Session.h"


Socks5::Socks5(WLSignal *wls) : WoklibPlugin(wls)
{
	sockid = 0;
	EXP_SIGHOOK("SOCKS5 Connect", &Socks5::Connect, 5);
}


Socks5::~Socks5()
{
	
}


int
Socks5::Connect(WokXMLTag *tag)
{
	new Socks5Session(wls,tag,sockid++);

	return 1;
}

