datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        connect.window.glade
				
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal


AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib  `pkg-config gtk+-2.0 libglade-2.0 --libs`

lib_LTLIBRARIES = \
	libconnectwindow.la

libconnectwindow_la_SOURCES = \
	GUIConnectWindow.cc  \
	GUIConnectWindow.h  \
	main.cc  \
	conn-window-manager.h  \
	conn-window-manager.cc

## File created by the gnome-build tools

