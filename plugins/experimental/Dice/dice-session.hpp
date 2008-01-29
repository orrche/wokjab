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

#ifndef _DICE_SESSION_HPP_
#define _DICE_SESSION_HPP_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class DiceSession: public WLSignalInstance 
{
public:
	DiceSession(WLSignal *wls, std::string session, std::string roomjid, GladeXML *parant_gxml);
	~DiceSession();
	
	void AddToSession(WokXMLTag &die);
	void Roll();
	
	int RandNR(WokXMLTag *tag);
	int Message(WokXMLTag *tag);
		
	static void DataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time, DiceSession *c);
	static void StartRoll(GtkButton *button, DiceSession *c);
	static void ClearHistory(GtkButton *button, DiceSession *c);
protected:

private:
	std::string session, roomjid, mynick;
	GladeXML *gxml, *parant_gxml;
	
	GtkListStore *roll_store, *hist_store;
	
	
	std::map <std::string, std::string> rand_num;
	std::map <std::string, WokXMLTag *> dice_data;
};

#endif // _DICE_SESSION_HPP_
