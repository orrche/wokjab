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

#ifndef _J_I_D_CONFIG_HPP_
#define _J_I_D_CONFIG_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;
class JIDConfig: public WoklibPlugin 
{
public:
	JIDConfig(WLSignal *wls);
	
	
	int Get(WokXMLTag *tag);
	int Store(WokXMLTag *tag);
	int Trigger(WokXMLTag *tag);
	int ReadConfig(WokXMLTag *tag);

	void SaveConfig();
	WokXMLTag * GetPosition(std::string path, std::string jid);
	
protected:
	WokXMLTag *config;
	std::map<std::string, WokXMLTag *> jidconfig;

private:
};

#endif // _J_I_D_CONFIG_HPP_
