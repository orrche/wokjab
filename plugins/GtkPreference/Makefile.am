libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -lwoklib -g -O3 -Wall
	 
	 
lib_LTLIBRARIES = \
	libgtkpreference.la 

libgtkpreference_la_SOURCES = \
	main.cc              \
	GtkPreference.cc              \
	GtkPreference.h              \
	GtkPString.h              \
	GtkPString.cc              \
	GtkPText.h              \
	GtkPText.cc              \
	GtkPBool.h              \
	GtkPBool.cc              \
	GtkPCommon.cc              \
	GtkPCommon.h              \
	GtkPPassword.h              \
	GtkPPassword.cc              \
	jid-list.hpp              \
	jid-list.cpp

libgtkpreference_la_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs`
	
datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        preference.jidlist.glade

EXTRA_DIST = ${pkgdata_DATA}

## File created by the gnome-build tools


