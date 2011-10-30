/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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

#ifndef __REGISTERSERVICE_H
#define __REGISTERSERVICE_H

#include <iostream>
#include <gtk/gtk.h>
#include <glade/glade.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class jep96 : public WoklibPlugin
{
	public:
		jep96(WLSignal *wls);
		~jep96();

		virtual std::string GetInfo() {return "Jep96 ( filetransfear )";};
		virtual std::string GetVersion() {return "0.0.1";};
		
		int SendFile(WokXMLTag *xml);
		int SendReply(WokXMLTag *xml);
		int JidMenu(WokXMLTag *xml);
		int JidMenuActivated(WokXMLTag *xml);
		int DiscoInfo(WokXMLTag *xml);
		int Wid(WokXMLTag *xml);
		int Connected(WokXMLTag *contag);
		int Terminated(WokXMLTag *contag);
		int Position(WokXMLTag *contag);
		int Finnished(WokXMLTag *contag);
		int Rejected(WokXMLTag *contag);
		int Accepted(WokXMLTag *contag);
		int MainMenu(WokXMLTag *tag);
		int Show(WokXMLTag *tag);
		int ReadConfig(WokXMLTag *tag);
		int FinishIgnore(WokXMLTag *tag);
		int FinishOpen(WokXMLTag *tag);
		int Incomming(WokXMLTag *tag);
		int NoHandleIncomming(WokXMLTag *tag);
		int FileAuth(WokXMLTag *tag);
	
		std::string PrettySize(unsigned long long size);
		void UpdateRowPosition(std::string sid);
		
		static gboolean Delete( GtkWidget *widget, GdkEvent *event, jep96 *c);
		static void CloseWindow(GtkButton *button, jep96 *c);
		static void RemoveStream(GtkButton *button, jep96 *c);
		static gboolean SpeedCalc (jep96 * c);
		static void DragGet(GtkWidget *wgt, GdkDragContext *context, GtkSelectionData *selection, guint info, guint time, jep96 *c);
	protected:
		std::string autoproxy;
		std::string autoproxytype;
		std::string openwith;
		std::string popup_ft_wid;
		WokXMLTag *config;

		guint speedcalctimerid;
		int sidnum;
		GladeXML *gxml;
		GtkWidget *filewindow;
		GtkListStore *file_store;
		
		std::map <std::string, GtkTreeRowReference*> rows;
		std::map <std::string, WokXMLTag *> sessions;
		
	
		enum 
		{
			FILE_ID_COLUMN,
			DATA_TRANSFEARED_COLUMN,
			STATUS_COLUMN,
			SID_COLUMN,
			SENDER_RECEIVER_COLUMN,
			RECEIVER_COLUMN,   // Unused
			SESSION_COLUMN, 
			EVENT_DATA_COLUMN,      // What is this ?!
			SENDING_COLUMN,   // True/False
			DATA_PROGRESS,
			TOOLTIP_COLUMN,
			NUM_COLUMNS
		};
};

#endif // __BROWSER_H
