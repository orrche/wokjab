/***************************************************************************
 *  Copyright (C) 2003-2004  Kent Gustavsson <oden@gmx.net>
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


#ifndef __WOKJAB_STATICFUNCTIONS_H
#define __WOKJAB_STATICFUNCTIONS_H
#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>
//#include <glib.h>

using namespace Woklib;


extern int read_data_from_socket(WokLib *sj, int socket);
extern void xml_start (void *data, const char *el, const char **attr);
extern void xml_charhndl (void *data, const char *string, int len);
extern void xml_end (void *data, const char *el);

#endif // __WOKJAB_STATICFUNCTIONS_H
