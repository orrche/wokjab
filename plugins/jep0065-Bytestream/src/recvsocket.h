/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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

#ifndef __RECV_SOCKET_H
#define __RECV_SOCKET_H


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;


class RecvSocket : public WLSignalInstance
{
	public:
		RecvSocket(WLSignal *wls, int socket);
		~RecvSocket();
	
		int ReadData(WokXMLTag *xml);
	private:
		int SendData(char *data, uint len);

		int socket;
		int pos;
		int stage;
		char data[300];
};





#endif // __RECV_SOCKET_H

