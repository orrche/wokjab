datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = 
				
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal


AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib  `pkg-config gtk+-2.0 libglade-2.0 --libs`

lib_LTLIBRARIES = \
	libguipluginwindow.la

libguipluginwindow_la_SOURCES = \
	GUIPluginWindow.cc     \
	GUIPluginWindow.h     \
	main.cc     \
	window-manager.cc     \
	window-manager.h

## File created by the gnome-build tools

