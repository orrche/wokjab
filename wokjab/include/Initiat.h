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



#ifndef _INITIAT_H_
#define _INITIAT_H_

#include "vector"

#include <Woklib/WokLibSignal.h>
#include <Woklib/WokLib.h>
#include <Woklib/WLSignal.h>
#include <gtk/gtk.h>

using namespace Woklib;

class Initiat : public WLSignalInstance
{
	public:
		Initiat(WLSignal *wls, WokLib *sj, int argc, char **argv);
		 ~Initiat();
	
		int Connected(WokXMLTag *xml);
		int AddListener(WokXMLTag *xml);
		int AddWatcher(WokXMLTag *xml);
		int AddErrorsock(GIOChannel *source, int socket);
		int Plugins(WokXMLTag *xml);
		int Version(WokXMLTag *xml);
		int GetArgs(WokXMLTag *xml);
		
		static gboolean input_callback(GIOChannel *source, GIOCondition condition, Initiat *c);

	protected:
		WokLib *sj;
		guint input_id;
		bool logged_in;
		std::map<int, std::string> sockets;
		std::map<int, std::string> freesockets;
		std::map<int, std::vector <guint> > errorsockets;
	
		WokXMLTag *args;
};


#endif	//_INITIAT_H_
