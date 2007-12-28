#
#  Copyright (C) 2003-2004  Kent Gustavsson <oden@gmx.net>
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


INCLUDES =
libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config --cflags glib-2.0`\
	 -Wall -Wimplicit -Wunused -Wmissing-prototypes\
	 -O1\
	 -g\
	-lwoklib


lib_LTLIBRARIES = libwoklib_groupchat.la

libwoklib_groupchat_la_SOURCES = \
	GroupChat.cpp\
	GroupChat.hpp \
	main.cc \
	room.hpp \
	room.cpp \
	user.hpp \
	user.cpp

libwoklib_groupchat_la_LDFLAGS = \
	`pkg-config --libs glib-2.0`

libwoklib_groupchat_la_LIBADD =
