libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall

lib_LTLIBRARIES = \
	libdisplaygtk.la 

libdisplaygtk_la_SOURCES = \
	main.cc           \
	DisplayGtk.cc           \
	DisplayGtk.h           \
	DisplayWidget.h           \
	DisplayWidget.cc           \
	InputDialog.h           \
	InputDialog.cc           \
	SignalGenDialog.cc           \
	SignalGenDialog.h 

		
AM_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs`
## File created by the gnome-build tools


