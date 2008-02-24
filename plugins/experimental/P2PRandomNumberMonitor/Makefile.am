libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -lwoklib -g -O3 -Wall

lib_LTLIBRARIES = \
	libp2pmonitor.la

libp2pmonitor_la_SOURCES = \
	main.cpp          \
	monitor.hpp          \
	monitor.cpp

datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
       p2prandmonitor.glade 
				
EXTRA_DIST = ${pkgdata_DATA}
## File created by the gnome-build tools

