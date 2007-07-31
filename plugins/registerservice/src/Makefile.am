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
	`pkg-config gtk+-2.0 --cflags` -lwoklib -g -O3 -Wall

lib_LTLIBRARIES = libregisterservice.la

libregisterservice_la_SOURCES = \
	main.cc\
	registerservice.cc\
	registerservice.h\
	registerservicewid.cc\
	registerservicewid.h

libregisterservice_la_LDFLAGS = \
	`pkg-config gtk+-2.0 --libs`

libregisterservice_la_LIBADD =
