libdir = $(prefix)/lib/woklib-plugins/secondary
INCLUDES = -I/usr/include/python2.4
AM_LDFLAGS = `python-config` -ldl -lpthread -lm -lutil
AM_CXXFLAGS =\
	-O3 -Wall
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

SUBDIRS = \
	preloader 

## File created by the gnome-build tools


