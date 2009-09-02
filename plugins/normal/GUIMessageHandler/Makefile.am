datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = 
				
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal


AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib  `pkg-config gtk+-2.0 libglade-2.0 --libs`

lib_LTLIBRARIES = \
	libguimessagehandler.la

libguimessagehandler_la_SOURCES = \
	GUIMessageHandler.cc     \
	GUIMessageHandler.h     \
	GUIMessageWidget.cc     \
	GUIMessageWidget.h     \
	main.cc

## File created by the gnome-build tools

