datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        GUIRoster.glade

EXTRA_DIST = ${pkgdata_DATA}
				

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS = `pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall -lwoklib
AM_LDFLAGS = `pkg-config libglade-2.0 gtk+-2.0 --libs`
	
lib_LTLIBRARIES = \
	libguiroster.la 

libguiroster_la_SOURCES = \
	main.cc          \
	GUIRoster.cc          \
	GUIRoster.h          \
	RosterItem.cc          \
	RosterItem.h 

## File created by the gnome-build tools


