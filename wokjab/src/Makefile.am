#
#  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

if DO_GTK

AM_LDFLAGS = -lwoklib  `pkg-config glib-2.0 gtk+-2.0 libglade-2.0 --libs`

bin_PROGRAMS = wokjab wokbot

wokbot_SOURCES = Initiat.cc bot.cc \
	plugin-loader.hpp plugin-loader.cpp wokjab.cc
	
	
wokjab_SOURCES = Initiat.cc main.cc \
	plugin-loader.hpp plugin-loader.cpp wokjab.cc
	
AM_CXXFLAGS = -I.. `pkg-config glib-2.0 gtk+-2.0 libglade-2.0 --cflags` -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST = wokjab.h bot.h

else

AM_LDFLAGS = -lwoklib `pkg-config glib-2.0 --libs` -ldl

bin_PROGRAMS = wokbot

wokbot_SOURCES = Initiat.cc bot.cc wokjab.cc\
	plugin-loader.hpp plugin-loader.cpp
	
	
wokjab_SOURCES = Initiat.cc main.cc \
	plugin-loader.hpp plugin-loader.cpp
	
AM_CXXFLAGS = -I.. `pkg-config glib-2.0 --cflags` -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST = wokjab.h bot.h

endif
