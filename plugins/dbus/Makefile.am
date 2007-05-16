libdir = $(prefix)/lib/woklib-plugins/normal

AM_CFLAGS =\
	`pkg-config dbus-glib-1 --cflags`

AM_CXXFLAGS =\
	`pkg-config dbus-glib-1 --cflags`

lib_LTLIBRARIES = \
	libdbus.la 

libdbus_la_SOURCES = \
	main.cc \
	WLDbus.cc \
	WLDbus.h \
	wokjab-dbus-glue.h \
	wokjab.xml
	

## File created by the gnome-build tools


