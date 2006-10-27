datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        jabberreversi.glade
				
EXTRA_DIST = ${pkgdata_DATA}

INCLUDES =
libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall

	
lib_LTLIBRARIES = \
	libreversi.la 

libreversi_la_SOURCES = \
	main.cc        \
	ReversiHandler.h        \
	ReversiHandler.cc        \
	Game.h        \
	Game.cc 
	
AM_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs`
	
SUBDIRS = \
	pixmaps \
	.
## File created by the gnome-build tools


