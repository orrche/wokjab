datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = 
				
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	 -O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib  


lib_LTLIBRARIES = \
	libxep0147.xmpp.uri.scheem.query.la

libxep0147_xmpp_uri_scheem_query_la_SOURCES = \
	main.cc                              \
	u-r-i-handler.hpp                              \
	u-r-i-handler.cpp

## File created by the gnome-build tools

