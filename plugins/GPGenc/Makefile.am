INCLUDES =
libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS =\
	-O3 -Wall -lwoklib -lgpgme `gpgme-config --cflags` 

lib_LTLIBRARIES = \
	libgpgenc.la

libgpgenc_la_SOURCES = \
	main.cc                \
	g-p-genc.hpp                \
	g-p-genc.cpp

libgpgenc_la_LDFLAGS = \
	`gpgme-config --libs`

## File created by the gnome-build tools

