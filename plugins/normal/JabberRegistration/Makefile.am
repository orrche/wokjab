datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = 
				
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	-O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib 

lib_LTLIBRARIES = \
	libjabberregistration.la

libjabberregistration_la_SOURCES = \
	main.cc                   \
	jabber-reg-handler.hpp                   \
	jabber-reg-handler.cpp                   \
	session.hpp                   \
	session.cpp

## File created by the gnome-build tools

