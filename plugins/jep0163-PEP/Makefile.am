libdir = $(prefix)/lib/woklib-plugins/normal

datadir = $(prefix)/lib/woklib/glade
pkgdata_DATA = \
        p-e-p-manager.glade

EXTRA_DIST = ${pkgdata_DATA}


AM_CXXFLAGS = `pkg-config libglade-2.0 gtk+-2.0 --cflags`

lib_LTLIBRARIES = \
	libjep0163-pep.la

libjep0163_pep_la_SOURCES = \
	main.cpp           \
	p-e-p-manager.hpp           \
	p-e-p-manager.cpp           \
	p-e-p-widget.hpp           \
	p-e-p-widget.cpp           \
	p-e-p-user.hpp           \
	p-e-p-user.cpp


libjep0163_pep_la_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs` -lwoklib

## File created by the gnome-build tools

