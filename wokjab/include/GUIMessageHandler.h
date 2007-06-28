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



#ifndef _GUIMESSAGEHANDLER_H_
#define _GUIMESSAGEHANDLER_H_

#include <Woklib/WLSignal.h>
#include <gtk/gtk.h>
#include <iostream>
#include "include/GUIMessageWidget.h"
#include "include/GUIWindowDock.h"

class GUIMessageHandler : public WLSignalInstance
{
	public:
		GUIMessageHandler(WLSignal *wls);
		 ~GUIMessageHandler();
	
		int new_message(WokXMLTag *tag);
		int OpenDialog(WokXMLTag *tag);
		int CloseDialog(WokXMLTag *tag);
		int Presence(WokXMLTag *tag);
		int JIDMenu( WokXMLTag *tag);
		int MenuOpenDialog( WokXMLTag *tag);
		int GetSpool(WokXMLTag *tag);
		int CopyBody(WokXMLTag *tag);

		void WriteToSpool(WokXMLTag *tag);
		void TriggerEvent(WokXMLTag *tag);
	protected:
		std::string SpoolDir;
		std::string msgicon;
		GdkPixbuf *pix_msg;
		GUIWindowDock *guiwd;
		WokXMLTag *XMLSpool;
		int id;
};


#endif	//_GUIMESSAGEHANDLER_H_
