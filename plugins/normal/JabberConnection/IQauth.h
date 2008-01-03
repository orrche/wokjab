/***************************************************************************
 *  Copyright (C) 2003-2004  Kent Gustavsson <oden@gmx.net>
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


#ifndef __WOKJAB_IQAUTH_H
#define __WOKJAB_IQAUTH_H

#include "Connection.h"
//#include "IQroster.h"

using namespace Woklib;

class IQauth : public WLSignalInstance
{
	public:
		IQauth (WLSignal *wls, std::string session, int con_type);
		~IQauth ();

		int xmlClearTextUser (WokXMLTag *tag);
		int SD_Challange(WokXMLTag *tag);
		int SD_Success(WokXMLTag *tag);
		int SD_Failure(WokXMLTag *tag);
		

		enum ConnectionType
		{
			ClearTextUser,
			SHA1User,
			HandshakeComponent,
			SASLDIGESTMD5,
			SASLPLAIN
		};
	
		
	private:
			void InitSHA1UserStage1();
			void InitSHA1UserStage2(WokXMLTag *tag);
			void InitClearTextUser();
			void InitHandshakeComponent();
			void InitSASLDIGESTMD5();
			void InitSASLPLAIN();
			std::string SD_A2(std::string digest_uri_value);
			std::string SD_A1(std::string username, std::string realm, std::string passwd, std::string nonce, std::string cnonce, std::string authzid);
			std::string HEX(std::string data);
			std::string H(std::string data);
			int con_type;
			std::string con_id;
			std::string session;
			std::string signal;
		
			std::string username;
			std::string server;
			std::string resource;
			std::string password;
};



#endif				// __WOKJAB_IQAUTH_H
