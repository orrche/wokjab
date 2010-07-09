libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config gtkmm-2.4 --cflags` -lwoklib -g -O3 -Wall
AM_LDFLAGS = -lwoklib  `pkg-config gtkmm-2.4 --libs`

lib_LTLIBRARIES = \
	libmassrename.la

libmassrename_la_SOURCES = \
	main.cpp    \
	MassRename.hpp    \
	MassRename.cpp
	
datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
       MassRename.xml 
				
EXTRA_DIST = ${pkgdata_DATA}
