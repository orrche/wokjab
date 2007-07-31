libdir = $(prefix)/lib/woklib-plugins/normal

datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        group-manager.glade

EXTRA_DIST = ${pkgdata_DATA}

AM_CFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags`

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -lwoklib
	
lib_LTLIBRARIES = \
	libgroupmanager.la

libgroupmanager_la_SOURCES = \
	main.cc             \
	group-manager.hpp             \
	group-manager.cpp             \
	g-m-widget.hpp             \
	g-m-widget.cpp

libgroupmanager_la_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs`
