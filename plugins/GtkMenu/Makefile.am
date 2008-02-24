

libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	`pkg-config gtk+-2.0 --cflags` -I../../../lib/woklib/include -g -O3 -Wall -lwoklib

lib_LTLIBRARIES = \
	libgtkmenu.la 

libgtkmenu_la_SOURCES = \
	main.cc        \
	gtkmenu.h        \
	gtkmenu.cc 

libgtkmenu_la_LDFLAGS = \
	`pkg-config gtk+-2.0 --libs`
	
## File created by the gnome-build tools


