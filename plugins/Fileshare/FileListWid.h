/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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
// Class: FileListWid
// Created by: Kent Gustavsson <nedo80@gmail.com>
//

#ifndef _FILE_LIST_WID_H_
#define _FILE_LIST_WID_H_
#include <gtk/gtk.h>
#include <glade/glade.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>


using namespace Woklib;

class FileListWid : public WLSignalInstance
{
	public:
		FileListWid(WLSignal *wls, WokXMLTag *xml, WokXMLTag *config);
		~FileListWid();

		int Auth(WokXMLTag *tag);
		int Finished(WokXMLTag *tag);
		void PopulateTree(WokXMLTag *tag, GtkTreeIter *iter);
		
		static void OpenFolder (GtkTreeView *tree_view, FileListWid *c);
		static void Download(GtkButton *button, FileListWid *c);
		static void SelectedDownload(GtkTreePath *path, FileListWid *c);
	protected:
		WokXMLTag *filelist;
		WokXMLTag *config;
		GladeXML *xml;
		std::string sid;
		std::string lsid;
		std::string session;
		std::string jid;
		
		GtkTreeStore *path_store;
		GtkListStore *folder_store;
};



#endif
