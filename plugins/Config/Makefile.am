INCLUDES =
libdir = $(prefix)/lib/woklib-plugins/system

AM_CXXFLAGS =\
	-I../../../lib/woklib/include -g -O3 -Wall

lib_LTLIBRARIES = \
	libconfig.la 

libconfig_la_SOURCES = \
	main.cc       \
	Config.cc       \
	Config.h 

pkgdata_DATA = \
	defaultconfig.xml
	
EXTRA_DIST = ${pkgdata_DATA}
	
## File created by the gnome-build tools


