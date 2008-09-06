datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        notification.control.glade
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -lwoklib -g -O3 -Wall

lib_LTLIBRARIES = \
	libnotificationwidget.la

libnotificationwidget_la_SOURCES = \
	main.cc                   \
	notification-manager.hpp                   \
	notification-manager.cpp                   \
	notification-widget.hpp                   \
	notification-widget.cpp

## File created by the gnome-build tools

