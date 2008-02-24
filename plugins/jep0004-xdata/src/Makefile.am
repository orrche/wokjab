#
#  Copyright (C) 2003-2006  Kent Gustavsson <oden@gmx.net>
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


AM_CXXFLAGS =\
	`pkg-config gtk+-2.0 --cflags`\
	-I.. \
	 -Wall -lwoklib

lib_LTLIBRARIES = libjep0004-xdata.la

libjep0004_xdata_la_SOURCES = \
	main.cc\
	WokJabxdatahandler.cc\
	WokJabxdataWidget.cc\
	xdatabase.cc\
	xdatasingletext.cc\
	xdatasingleprivate.cc\
	xdatamultitext.cc\
	xdatafixed.cc\
	xdataunknown.cc\
	xdatabool.cc\
	xdatahidden.cc\
	xdatasinglelist.cc\
	xdatamultilist.cc
	
libjep0004_xdata_la_LDFLAGS = 	`pkg-config gtk+-2.0 --libs`

libjep0004_xdata_la_LIBADD =
