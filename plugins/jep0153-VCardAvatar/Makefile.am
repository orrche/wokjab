#
#  Copyright (C) 2003-2008 Kent Gustavsson <oden@gmx.net>
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


libdir = $(prefix)/lib/woklib-plugins/normal

datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        vcardavatar.glade
        
AM_CFLAGS =\
	`pkg-config gtk+-2.0 --cflags`

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -lwoklib

	
lib_LTLIBRARIES = libjep0153-vcardavatar.la 

libjep0153_vcardavatar_la_SOURCES = \
	main.cc                   \
	VCardAvatar.cc                   \
	VCardAvatar.h                   \
	User.cc                   \
	User.h 



libjep0153_vcardavatar_la_LDFLAGS = \
	`pkg-config gtk+-2.0 openssl --libs` 
