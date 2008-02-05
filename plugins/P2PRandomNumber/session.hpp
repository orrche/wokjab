/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
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

#ifndef _SESSION_HPP_
#define _SESSION_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace Woklib;

class Session: public WLSignalInstance 
{
public:
	Session(WLSignal *wls, WokXMLTag *xml);
	~Session();
		
	int Message(WokXMLTag *tag);
	int Timeout(WokXMLTag *tag);
	
	bool Is(const std::string &in_owner, const std::string &in_id, const std::string &session, const std::string &roomjid);
	void GetData(WokXMLTag &tag);
	void Cancel();
protected:
	std::string id;
	std::string owner;
	WokXMLTag origxml;
	std::string mynick;
	std::string myplain;
	bool sent_plain;
	bool deleted;
	
	std::map <std::string, std::string> hashes;
	std::map <std::string, std::string> plain;
	std::map <std::string, std::string> numbers;
	
	std::list <std::string> requiredusers;
private:

};

#endif // _SESSION_HPP_
