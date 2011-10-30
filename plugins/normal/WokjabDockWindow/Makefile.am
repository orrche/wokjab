datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = 
				
EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config gtkmm-2.4 --cflags` -O3 -Wall -lwoklib
AM_LDFLAGS = -lwoklib  `pkg-config gtkmm-2.4 --libs`


lib_LTLIBRARIES = \
	libwokjabdockwindow.la

libwokjabdockwindow_la_SOURCES = \
	main.cc                 \
	wokjab-dock-window.hpp                 \
	wokjab-dock-window.cpp                 \
	wokjab-dock-window-handler.hpp                 \
	wokjab-dock-window-handler.cpp                 \
	WokjabDockWindowMaster.cpp						\
	WokjabDockWindowMaster.h

## File created by the gnome-build tools

