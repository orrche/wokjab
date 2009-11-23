/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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



#ifndef _GUIMESSAGEWIDGET_H_
#define _GUIMESSAGEWIDGET_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>
#include <gtk/gtk.h>

#include <map>

using namespace Woklib;

class GUIMessageWidget : public WLSignalInstance
{
	public:
		GUIMessageWidget(WLSignal *wls, std::string session, std::string from, int id);
		 ~GUIMessageWidget();
	
		int NewMessage(WokXMLTag *tag);
	
		int PutText(GtkTextIter *iter, WokXMLTag &message);
		int Message(WokXMLTag &message);
		int Message(WokXMLTag &message, std::string jid);

		int Presence(WLSignalData *pd);
		static gboolean key_press_handler(GtkWidget * widget, GdkEventKey * event,
						 gpointer user_data);
		void own_message(std::string msg);
		int InsertCommand(WokXMLTag &tag);

		int Config(WokXMLTag *tag);
		int NewPresence(WokXMLTag *tag);
		int Activate(WokXMLTag *tag);
		int Close(WokXMLTag *tag);
		int NewEvent(WokXMLTag *tag);
		int RemoveEvent(WokXMLTag *tag);
		int SentMessage(WokXMLTag *tag);
		int UserActivities(WokXMLTag *tag);
	
		
		void SetLabel();
	protected:
		static gboolean focus_event (GtkWidget *widget, GdkEventFocus *event, GUIMessageWidget *c);
		static gboolean expander_activate (GtkExpander *expander, gpointer user_data);
		static void SizeAllocate (GtkWidget *widget, GtkRequisition *requisition,gpointer user_data);
		static gboolean widget_destroy(GtkWidget *widget, GdkEvent *event, gpointer user_data);
		static gboolean DragDest(GtkWidget *widget, GdkDragContext *dc,gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, GUIMessageWidget *data);
		static gboolean DragDrop(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint time, GUIMessageWidget *c);
		static gboolean Menu(GtkButton *button, GdkEventButton *event, GUIMessageWidget *data);
		static void Destroy(GtkWidget *widget, GUIMessageWidget *c);
		static void LDestroy(GtkWidget *widget, GUIMessageWidget *c);
		static gboolean Scroll (GtkWidget *widget, GdkEventScroll *event, GUIMessageWidget *c);
		static gboolean tw1_event_after (GtkWidget *text_view, GdkEvent  *ev, GUIMessageWidget *c);
		static gboolean key_press_event(GtkWidget * widget, GdkEventKey * event, GUIMessageWidget *c);
		static gboolean CommandExec(GtkWidget *button, GdkEventButton *event, GUIMessageWidget *c);
	
		void HookSignals();
		void UnHookSignals();

		std::string GetTimeStamp(WokXMLTag *tag);
		
		WokXMLTag *config;
		GtkWidget *expander;
		GtkWidget *vbox;
		GtkWidget *tophbox;
		GtkWidget *incomming_box;
		GtkWidget *scroll1;
		GtkWidget *textview1;
		GtkWidget *scroll2;
		GtkWidget *textview2;
		GtkWidget *label_hbox;
		GtkWidget *label_label;
		GtkWidget *label_image;
		GtkWidget *jid_label;
		GtkWidget *label;
		GtkWidget *vpaned;
		GtkWidget *image;
		GtkWidget *mainwindowplug;
		GtkWidget *labelplug;	
		GtkWidget *activitybox;
		GtkWidget *activitytable;
		GdkPixbuf *pix_msg;
		GdkColor color_red;
		GtkTextBuffer *buffer1;	
		GtkTextBuffer *buffer2;
		GtkTextMark *end_mark;
	
		std::string msgicon;
		std::string xml_nick;
		std::string xml_jid;
		std::string xml_msg;
		time_t xml_time;
		
		std::string session;
		std::string from;
		std::string from_no_resource;
		std::string nick;
		std::string resource;
		
		bool secondmessageother; // Shows if the user has sent more then one message for continue words..
		bool secondmessageme;
		bool focus;
		bool msg_waiting;
		bool hasresource;
		
		double fontsize;
		
		std::list < std::string> eventidlist;
		std::map<std::string, GtkTextTag*> tags;
		int cmd_count;
		std::map<int, WokXMLTag *> commands;
		std::map<GtkWidget *, WokXMLTag*> event_list;
		std::list<GtkWidget *> event_list_order;
};


#endif	//_GUIMESSAGEWIDGET_H_
