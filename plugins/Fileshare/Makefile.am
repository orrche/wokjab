libdir = $(prefix)/lib/woklib-plugins/normal
datadir = $(prefix)/lib/woklib/glade

pkgdata_DATA = \
        FileListWid.glade \
	SearchWid.glade

EXTRA_DIST = ${pkgdata_DATA}

								
AM_CXXFLAGS =\
	`pkg-config libglade-2.0 gtk+-2.0 --cflags` -O3 -Wall -lsqlite3 -g -pg -lwoklib
	

lib_LTLIBRARIES = \
	libfileshare.la 
	
libfileshare_la_SOURCES = \
	main.cc \
	FileShare.cc \
	FileShare.h \
	FileListWid.cc \
	FileListWid.h \
	File.cc \
	File.h \
	Download.cc \
	Download.h \
	SearchWid.cc \
	SearchWid.h \
	download-queue.hpp \
	download-queue.cpp
	
libfileshare_la_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs` -lz

## File created by the gnome-build tools


