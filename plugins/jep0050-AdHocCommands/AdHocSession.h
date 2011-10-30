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
// Class: AdHocSession
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Oct 16 15:18:51 2006
//

#ifndef _ADHOCSESSION_H_
#define _ADHOCSESSION_H_

#include <gtk/gtk.h>
#include <glade/glade.h>


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class AdHocSession : public WLSignalInstance
{
	public:
		AdHocSession(WLSignal *wls, WokXMLTag *tag);
		 ~AdHocSession();
	
		int XDataResponse(WokXMLTag *tag, std::string button);
		
		int ExecResponse(WokXMLTag *tag);
		
		static void Button(GtkButton *button, AdHocSession *c);
		static void CancelButton(GtkButton *button, AdHocSession *c);
		static void CloseButton(GtkButton *button, AdHocSession *c);
		static void Destroy(GtkWidget *widget, AdHocSession *c);
	protected:
		std::map< GtkWidget *, std::string > buttons;
		GladeXML *xml;
		GtkWidget *container;
		
		std::string xdataid;
		std::string sessionid;
		std::string session;
		std::string status;
		std::string node;
		std::string jid;
		int state;
};


#endif	//_ADHOCSESSION_H_

