libdir = $(prefix)/lib/woklib-plugins/secondary
INCLUDES = $(PY_CFLAGS)
AM_LDFLAGS = $(PY_LIBS) -ldl -lpthread -lm -lutil
AM_CXXFLAGS =\
	-O3 -Wall -lwoklib
lib_LTLIBRARIES = \
	libwokpython.la 

libwokpython_la_SOURCES = \
	main.cc          \
	WokPython.h          \
	WokPython.cc          \
	PyScript.h          \
	PyScript.cc          \
	PyWXMLTag.h          \
	PyWXMLTag.cc          \
	PySig.h          \
	PySig.cc 

libwokpython_la_LDFLAGS = \
        `pkg-config gtk+-2.0 --libs` $(PY_LIBS) -ldl -lpthread -lm -lutil

SUBDIRS = \
	preloader 

## File created by the gnome-build tools


