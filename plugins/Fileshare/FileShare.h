/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: FileShare
// Created by: Kent Gustavsson <nedo80@gmail.com>
//

#ifndef _FILE_SHARE_H_
#define _FILE_SHARE_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <sqlite3.h>

using namespace Woklib;

/**
 * To share files nicely in wokjab
 */
class FileShare : public WoklibPlugin
{
	public:
		FileShare(WLSignal *wls);
		~FileShare();
	
		virtual std::string GetInfo() {return "Fileshare";};
		virtual std::string GetVersion() {return VERSION;};
	
		int FileListResponse(WokXMLTag *tag);
		int ListRequest(WokXMLTag *tag);
		int MainMenu(WokXMLTag *tag);
		int Menu(WokXMLTag *tag);
		int View(WokXMLTag *tag);
		int IncommingFilelist(WokXMLTag *tag);
		int Rebuild(WokXMLTag *tag);
		int IncommingFile(WokXMLTag *tag);
		int Config(WokXMLTag *tag);
		int OpenSearchWid(WokXMLTag *tag);
		int Search(WokXMLTag *xml);
		int IncommingSearch(WokXMLTag *tag);
		int MenuDownload(WokXMLTag *tag);
		int Commands(WokXMLTag *tag);


		bool HasPermission(std::string session, std::string jid);
		int Com_Search(WokXMLTag *tag);

		void PopulateTree(WokXMLTag *tag, std::string dir, std::string virt_dir);
		WokXMLTag * AddFolder(WokXMLTag *point, std::string name);
		
		static int sql_callback(FileShare *c, int argc, char **argv, char **azColName);
		static int sql_callback_search(FileShare *c, int argc, char **argv, char **azColName);
	protected:
		WokXMLTag *config;
		WokXMLTag *search_result;
		int fileshareid;
		std::string path;
		std::string filetosend;
		sqlite3 *db;
		std::map <std::string, std::string> sharepoints;
		int n_id;

};

#endif

