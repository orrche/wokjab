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

#ifndef _XDATA_CONFIG_SESSION_H_
#define _XDATA_CONFIG_SESSION_H_


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>


using namespace Woklib;


class xdataConfigSession: public WLSignalInstance 
{
public:
	xdataConfigSession(WLSignal *wls, WokXMLTag &tag, std::string id);

	std::string GetID();
	void Execute(WokXMLTag *tag);
	void SendResponse(std::string iq_id);
	void PopulateXML(WokXMLTag &xdata, WokXMLTag &config, std::string = "");
	void SetConfigParamRec(WokXMLTag &tag, const std::string &path, WokXMLTag &field);
	
	WokXMLTag * GetPosition(WokXMLTag *place);
protected:
	std::string myID;
	std::string path;

	WokXMLTag origxml;
	WokXMLTag data;
private:

};

#endif // _XDATA_CONFIG_SESSION_H_
