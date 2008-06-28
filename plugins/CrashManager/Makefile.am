libdir = $(prefix)/lib/woklib-plugins/normal


datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        cmgui.glade

EXTRA_DIST = ${pkgdata_DATA}


AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` 

lib_LTLIBRARIES = \
	libcrashmanager.la

libcrashmanager_la_SOURCES = \
	main.cc             \
	crash-manager.hpp             \
	crash-manager.cpp             \
	c-m-g-u-i.hpp             \
	c-m-g-u-i.cpp

libcrashmanager_la_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs` -lwoklib

## File created by the gnome-build tools

