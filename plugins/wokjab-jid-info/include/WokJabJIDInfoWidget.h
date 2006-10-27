/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef _WOKJABJIDINFOWIDGET_H_
#define _WOKJABJIDINFOWIDGET_H_

#include <gtk/gtk.h>
#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

/**
 * Widget handler for showing information about a specific jid 
 * Rather basic rather ugly
 */
class WokJabJIDInfoWidget : public WLSignalInstance
{
	public:
		WokJabJIDInfoWidget(WLSignal *wls, const std::string &jid, const std::string &session);
		 ~WokJabJIDInfoWidget();
		
		virtual std::string GetInfo() {return "Jid Info";};
		virtual std::string GetVersion() {return "0.0.1";};
	
		int xml (WokXMLTag *tag);
		int vcard (WokXMLTag *tag);
	protected:
		GtkWidget *BirthdayLabel;
		GtkWidget *BirthdayLabelText;
		GtkWidget *NicknameLabel;
		GtkWidget *NicknameLabelText;
		GtkWidget *FullnameLabel;	
		GtkWidget *FullnameLabelText;
		GtkWidget *WebsiteLabel;
		GtkWidget *WebsiteLabelText;
		GtkWidget *ClientLabel;
		GtkWidget *ClientLabelText;
		GtkWidget *VersionLabel;
		GtkWidget *VersionLabelText;
		GtkWidget *OSLabel;
		GtkWidget *OSLabelText;
	
		GtkWidget *textview1;
	
		std::string id_version;
		std::string id_vcard;
		std::string session, jid;
};


#endif	//_WOKJABJIDINFOWIDGET_H_
