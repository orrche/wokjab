/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
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

#ifndef _JABBER_REG_HANDLER_HPP_
#define _JABBER_REG_HANDLER_HPP_

#include <gtk/gtk.h>
#include <glade/glade.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>


using namespace Woklib;
class JabberRegHandler;

#include "session.hpp"


class JabberRegHandler: public WoklibPlugin 
{
public:
	JabberRegHandler(WLSignal *wls);
		
	int Init(WokXMLTag *tag);
	int NewSession(WokXMLTag *tag);
	void Remove(std::string session);
		
	int MenuAction(WokXMLTag *tag);
	int Menu(WokXMLTag *tag);
	int XDataResp(WokXMLTag *tag);
protected:

	std::map<std::string, Session*> sessions;
private:

};

#endif // _JABBER_REG_HANDLER_HPP_
