/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
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

#include "crash-manager.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>

#include <gtk/gtk.h>

namespace sig_linux_signal
{
	#include <signal.h>
}

// Hope this call is threadsafe now 
void serial_handler (int status) {
	gtk_main_quit();
}


CrashManager::CrashManager(WLSignal *wls) : WoklibPlugin(wls)
{
	exiting_cleanly = false;
	EXP_SIGHOOK("Display Signal", &CrashManager::Sig, 1000);
	EXP_SIGHOOK("Program Exit", &CrashManager::Exit, 1000);
	EXP_SIGHOOK("GetMenu", &CrashManager::Menu, 1000);
	EXP_SIGHOOK("Jabber GUI CrashManager OpenWid", &CrashManager::Wid, 1000);
	
	std::stringstream fn;
	fn << g_get_home_dir() << "/.wokjab/sig." << getpid() << ".log";
	filename = fn.str();
	file.open(filename.c_str(), std::ios::out);
}


CrashManager::~CrashManager()
{
	if ( file )
		file.close();
	unlink(filename.c_str());
	
	std::list <CMGUI*>::iterator iter;
	for ( iter = widgets.begin() ; iter != widgets.end() ; iter++)
	{
		delete *iter;		
	}
}

int
CrashManager::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Crash Manager");
	item.AddAttr("signal", "Jabber GUI CrashManager OpenWid");
	return 1;
}

int
CrashManager::Wid(WokXMLTag *tag)
{
	widgets.push_back(new CMGUI(wls, this));
	return 1;
}

void
CrashManager::Remove(CMGUI *wid)
{
	std::list <CMGUI*>::iterator iter;
	if ( ( iter = std::find(widgets.begin() , widgets.end() , wid) ) != widgets.end() )
	{
		widgets.erase(iter);
		delete wid;
	}
}

int
CrashManager::Exit(WokXMLTag *tag)
{
	exiting_cleanly = true;
	file.close();
	unlink(filename.c_str());
	return 1;
}

int
CrashManager::Sig(WokXMLTag *tag)
{
	if ( exiting_cleanly )
		return 1;
	if ( tag->GetAttr("level") == "0")
	{
		file.close();
		file.open(filename.c_str(), std::ios::out);
	}
	
	for( int n = atoi(tag->GetAttr("level").c_str()) ; n ; n-- )
		file.write("  ",2);
	file.write(tag->GetAttr("name").c_str(), tag->GetAttr("name").size());
	file.write("\n",1);
	file.flush();
	return 1;
}
