/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2009 <nedo80@gmail.com>
 * 
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _XDATA_CONFIG_H_
#define _XDATA_CONFIG_H_


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "xdata-config-session.h"

using namespace Woklib;

class xdataConfig: public WoklibPlugin 
{
public:
	xdataConfig(WLSignal *wls);
	~xdataConfig();
	
	int Commands(WokXMLTag *tag);
	int configure_request(WokXMLTag *tag);
	int ReadConfig(WokXMLTag *tag);
	
	bool HasPermission(std::string session, std::string jid);
protected:
	WokXMLTag *config;
	int id;
	std::map <std::string, xdataConfigSession *> sessions;
private:

};

#endif // _XDATA_CONFIG_H_
