/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifndef _SOCKS5_HOST_SESSION_HPP_
#define _SOCKS5_HOST_SESSION_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>
#include "map"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define BUFFERSIZE 300
using namespace Woklib;

class Socks5HostSession: public WLSignalInstance 
{
public:
	Socks5HostSession(WLSignal *wls, WokXMLTag &tag, int in_socket);
	~Socks5HostSession();
	
	int ReadData(WokXMLTag *xml);
	int SendData(char *data, uint len);
protected:
	WokXMLTag settings;
	int socket;
	int pos,stage;
	char data[BUFFERSIZE];
private:

};

#endif // _SOCKS5_HOST_SESSION_HPP_
