libdir = $(prefix)/lib/woklib-plugins/normal

datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        pubsub.xml

EXTRA_DIST = ${pkgdata_DATA}


AM_CXXFLAGS = `pkg-config libglade-2.0 gtk+-2.0 --cflags`

lib_LTLIBRARIES = \
	libgtk.jep0060.pubsub.la

libgtk_jep0060_pubsub_la_SOURCES = \
	main.cpp                  \
	pub-sub-manager.hpp                  \
	pub-sub-manager.cpp                  \
	pub-sub-widget.hpp                  \
	pub-sub-widget.cpp

libgtk_jep0060_pubsub_la_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs` -lwoklib

## File created by the gnome-build tools

