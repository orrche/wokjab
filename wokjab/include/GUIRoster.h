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



#ifndef _GUIROSTER_H_
#define _GUIROSTER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>
#include <gtk/gtk.h>

class GUIRoster;

using namespace Woklib;

class GUIRoster:public WLSignalInstance
{
    public:
	GUIRoster (WLSignal *wls);
	~GUIRoster ();

	int View(WokXMLTag *xml);
	int GetIcon(WokXMLTag *xml);
	int DeleteMenu(WokXMLTag *xml);
	int DeleteAction(WokXMLTag *xml);
	

	static void ToggleButton(GtkWidget *widget, GUIRoster *c);
	
	void SaveConfig (bool connect_sig = true);
      protected:

	int ReadConfig (WokXMLTag *tag);
	
	std::map <GtkWidget *, int> bstate;
	std::map <std::string, std::string> icons;
	  
	GtkWidget *hbb;
	GtkWidget *togg1;
	GtkWidget *togg2;
	GtkWidget *togg3;
	WokXMLTag *config;
};


#endif //_GUIROSTER_H_l
