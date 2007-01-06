/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: AdHocWid
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Oct 15 21:24:22 2006
//

#ifndef _ADHOCWID_H_
#define _ADHOCWID_H_


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

using namespace Woklib;

class AdHocWid : public WLSignalInstance
{
	public:
		AdHocWid(WLSignal *wls, std::string session, std::string jid, std::string node);
		 ~AdHocWid();
	
		int Set(WokXMLTag *tag);
		int ExecResponse(WokXMLTag *tag);
		
		void Exec();
		
		static void ExecButton(GtkButton *button, AdHocWid *);
		static void Destroy(GtkWidget *widget, AdHocWid *c);
		static void CloseButton(GtkButton *button, AdHocWid *c);
	protected:
		GtkListStore *model;
		GladeXML *xml;
		std::string session;

};

enum
{
	NAME_COLUMN = 0,
	COMMAND_COLUMN,
	JID_COLUMN,
	NUM_COLUMNS
};

#endif	//_ADHOCWID_H_

