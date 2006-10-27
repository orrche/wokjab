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
// Class: AdHoc
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Oct 15 01:03:03 2006
//

#ifndef _ADHOC_H_
#define _ADHOC_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class AdHoc : public WoklibPlugin
{
	public:
		AdHoc(WLSignal *wls);
		 ~AdHoc();
	
		int Feature(WokXMLTag *tag);
		int Menu(WokXMLTag *tag);
		int Start(WokXMLTag *tag);
		int CommandExec(WokXMLTag *tag);
	protected:
	
	
};


#endif	//_ADHOC_H_

