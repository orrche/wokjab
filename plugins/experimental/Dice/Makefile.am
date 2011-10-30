libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -lwoklib -g -O3 -Wall
AM_LDFLAGS = -lwoklib  `pkg-config gtk+-2.0 libglade-2.0 --libs`

lib_LTLIBRARIES = \
	libdice.la

libdice_la_SOURCES = \
	main.cpp    \
	dice.hpp    \
	dice.cpp    \
	dice-session.hpp    \
	dice-session.cpp    

## File created by the gnome-build tools

datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
       dice.glade 
				
EXTRA_DIST = ${pkgdata_DATA}
