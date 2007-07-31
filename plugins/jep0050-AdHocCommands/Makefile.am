datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        AdHocWid.glade \
	AdHocSession.glade
EXTRA_DIST = ${pkgdata_DATA}

INCLUDES =
libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall -lwoklib
	
lib_LTLIBRARIES = \
	libjep0050-adhoc.la 

libjep0050_adhoc_la_SOURCES = \
	main.cc      \
	AdHoc.cc      \
	AdHoc.h      \
	AdHocWid.cc      \
	AdHocWid.h      \
	AdHocSession.cc      \
	AdHocSession.h 

AM_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs`

## File created by the gnome-build tools


