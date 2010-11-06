/***************************************************************************
 *  Copyright (C) 2003-2010  Kent Gustavsson <nedo80@gmail.com>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <iostream>

#include "wokjab.h"
#include "../include/main.h"
#include <glib-2.0/glib.h>

using std::string;
using std::cout;
using std::endl;

int
main (int argc, char **argv)
{

 	// Hum how ugly but what isn't in the main program of wokjab?
	//gtk_init_check(&argc, &argv);
	//g_init_check(&argc, &argv);
	Wokjab wj(argc, argv, "wokbot");
	
	GMainLoop *main_loop = g_main_loop_new (NULL , FALSE);   
	g_main_loop_run (main_loop);
	
	return (0);
}
