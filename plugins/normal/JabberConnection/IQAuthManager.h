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
// Class: IQAuthManager
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Nov 21 20:53:53 2006
//

#ifndef _IQAUTHMANAGER_H_
#define _IQAUTHMANAGER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class IQAuthManager : public WLSignalInstance
{
	public:
		IQAuthManager(WLSignal *wls);
		 ~IQAuthManager();
	
		int BindResp(WokXMLTag *tag);
		int Feature(WokXMLTag *tag);
	
	protected:
		// IQAuthManager variables
	
		// TODO: add member variables...
	
};


#endif	//_IQAUTHMANAGER_H_

