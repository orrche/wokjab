/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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
// Class: Config
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Jun 10 02:25:01 2005
//

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <iostream>

#include <Woklib/WokLib.h>

using namespace Woklib;

/**
 * Handles the config part of each module in a more or less generic way should 
 * recode to remove a redundant config tag in the returned xml 
 */
class Config : public WoklibPlugin
{
	public:
		Config(WLSignal *wls);
		 ~Config();
	
		virtual std::string GetInfo() {return "XML Config";};
		virtual std::string GetVersion() {return "0.0.1";};
	
		WokXMLTag *GetPosition(std::string path);
		void Load();
		
		int Init(WokXMLTag *tag);
		int Store(WokXMLTag *tag);
		int Trigger(WokXMLTag *tag);
		int Save(WokXMLTag *tag);
		int GetTree(WokXMLTag *tag);
	protected:
		WokXMLTag *basetag;
		WokXMLTag *truebasetag;
		std::string filename;
};


#endif	//_CONFIG_H_

