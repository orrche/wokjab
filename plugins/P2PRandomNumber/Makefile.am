AM_CXXFLAGS = -lwoklib `pkg-config openssl --cflags`


libdir = $(prefix)/lib/woklib-plugins/normal

lib_LTLIBRARIES = \
	libp2prandomnumber.la

libp2prandomnumber_la_SOURCES = main.cpp \
	rand.hpp\
	rand.cpp\
	session.hpp\
	session.cpp

libp2prandomnumber_la_LDFLAGS = \
	`pkg-config openssl --libs`

SUBDIRS = \
	.

## File created by the gnome-build tools

