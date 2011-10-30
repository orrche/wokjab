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
// Class: ScriptLoader
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Mar 21 20:32:31 2006
//

#ifndef _SCRIPTLOADER_H_
#define _SCRIPTLOADER_H_
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class ScriptLoader : public WoklibPlugin
{
	public:
		ScriptLoader(WLSignal *wls);
		 ~ScriptLoader();

		virtual std::string GetInfo() {return "Script Loader";};
		virtual std::string GetVersion() {return VERSION;};
	
		int Menu(WokXMLTag *tag);
		int ShowWidget(WokXMLTag *tag);
		int Config(WokXMLTag *tag);
		
		void UpdateList();

		static void Destroy( GtkWidget *widget, ScriptLoader *c );
		static void Reload(GtkButton *button, ScriptLoader *c);
		static void Unload(GtkButton *button, ScriptLoader *c);
		static void Load(GtkButton *button, ScriptLoader *c);
		static void LoadButton(GtkButton *button, ScriptLoader *c);
	protected:
		GladeXML *xml;
		GtkListStore *script_store;
		GtkWidget *file_selector;
		
		WokXMLTag *config;
};


#endif	//_SCRIPTLOADER_H_

