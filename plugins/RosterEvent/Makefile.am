datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        roster-event.glade

EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS = `pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall
AM_LDFLAGS = `pkg-config libglade-2.0 gtk+-2.0 --libs`

lib_LTLIBRARIES = \
	librosterevent.la 

librosterevent_la_SOURCES = \
	main.cpp           \
	roster-event.cpp           \
	roster-event.hpp 

## File created by the gnome-build tools


