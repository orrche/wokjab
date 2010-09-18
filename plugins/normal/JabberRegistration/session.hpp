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

#ifndef _SESSION_HPP_
#define _SESSION_HPP_


#include <gtk/gtk.h>
#include <glade/glade.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class Session;

#include "jabber-reg-handler.hpp"


class Session: public WLSignalInstance 
{
public:
	Session(WLSignal *wls, WokXMLTag &origxml, JabberRegHandler *father);
	~Session();

	int Connect(WokXMLTag *tag);
	int RegTagResp(WokXMLTag *tag);
	int Finnish(WokXMLTag *tag);
protected:
	WokXMLTag *origxml;
	JabberRegHandler *father;
private:

};

#endif // _SESSION_HPP_
