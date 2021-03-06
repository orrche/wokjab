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
// Class: GroupChatTooltip
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Oct 12 14:00:25 2006
//

#ifndef _GROUPCHATTOOLTIP_H_
#define _GROUPCHATTOOLTIP_H_


#include <map>

#include <glade/glade.h>
#include <gtk/gtk.h>
#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class GroupChatTooltip : public WLSignalInstance
{
	public:
		GroupChatTooltip(WLSignal *wls);
		 ~GroupChatTooltip();
	
		static gboolean Expose(GtkWidget *widget, GdkEventExpose *event, GroupChatTooltip *c);
		static gboolean  DispWindow (GroupChatTooltip * c);
		int Set(WokXMLTag &tag);
		int Reset(WokXMLTag &tag);
	protected:
		WokXMLTag *currenttag;
		GladeXML *xml;
		guint id;
	
};


#endif	//_GROUPCHATTOOLTIP_H_

