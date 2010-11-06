datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = 
				
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	-O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib 

lib_LTLIBRARIES = \
	libatomnotifications.la

libatomnotifications_la_SOURCES = \
	main.cc                  \
	atom-handler.hpp                  \
	atom-handler.cpp

## File created by the gnome-build tools

