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

#ifndef __JEP_65_SEND_H
#define __JEP_65_SEND_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "fstream"

#define SHUNKSIZE 1000000
//#define SHUNKSIZE 10

using namespace Woklib;

/**
 * Used for transmitting files 
	*/
	
class jep65send : public WLSignalInstance
{
	public:
		jep65send(WLSignal *wls, WokXMLTag *xml, std::string sport);
		~jep65send();
		
		virtual std::string GetInfo() {return "Jep65 ( filetransfear )";};
		virtual std::string GetVersion() {return "0.0.1";};
	
		const std::string & GetHash();
	
		void ReqProxy();
		void SendInitiat( bool use_proxy);
		void InitProxy();

		int InitProxyReply(WokXMLTag *tag);
		int ProxyReply(WokXMLTag *tag);
		int FileTransfear(WokXMLTag *tag);
		int InitProxyConnect(WokXMLTag *tag);
		int SocketAvailibule( WokXMLTag *tag);
		int SOCKS_Established(WokXMLTag *tag);
		int SOCKS_Data(WokXMLTag *tag);
		int TransfearStart(WokXMLTag *tag);
		int Timeout(WokXMLTag *tag);
			
		int SendData(char *data, uint len);
	private:
		WokXMLTag *data;
		std::string proxy;
		std::string me;
		std::string hash;
		std::string file;
		std::string iqid;
		std::string session;
		std::string sid;
		std::string to;
		std::string sport;
		
		std::string pjid;
		std::string phost;
		std::string pzeroconf;
		std::string pport;
		
		int socket;
		int rate;
		int baserate;
		unsigned long long size, fsize;
		int fbend;
		int fbpos;
	
		char filebuf[SHUNKSIZE];
	
		bool throttled;
	
		std::ifstream ffile;
};


#endif // __JEP_65_SEND_H

