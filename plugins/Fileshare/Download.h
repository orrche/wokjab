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
// Class: Download
// Created by: Kent Gustavsson <nedo80@gmail.com>
//

#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "File.h"
#include <vector>
#include <list>

using namespace Woklib;

class DownloadFolder : public WLSignalInstance
{
	public:
		DownloadFolder(WLSignal *wls, WokXMLTag *folder, std::string jid, std::string session, std::string d_path);
		~DownloadFolder();

		void DownloadFile(std::vector<std::string> vect);
		void GetFiles(std::string path, WokXMLTag *tag);
		
		int Finished(WokXMLTag *tag);
	protected:
		std::string d_path;
		WokXMLTag *folder;
		std::string jid;
		std::string session;
		std::string sid;
		
		File *f;
	
		std::list<std::vector<std::string> > list;
};


#endif // _FILE_H
