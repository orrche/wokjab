datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        tooltip.glade
				
EXTRA_DIST = ${pkgdata_DATA}

INCLUDES =
libdir = $(prefix)/lib/woklib-plugins/normal


AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib  `pkg-config gtk+-2.0 libglade-2.0 --libs`

lib_LTLIBRARIES = \
	libtooltip.la 

libtooltip_la_SOURCES = \
	main.cc        \
	Tooltip.h        \
	Tooltip.cc 

## File created by the gnome-build tools


