libdir = $(prefix)/lib/woklib-plugins/secondary
kentmoduledir = $(prefix)/lib

AM_CXXFLAGS =\
	-O3 -Wall -lwoklib $(PY_CFLAGS)
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
        `pkg-config gtk+-2.0 --libs` $(PY_LIBS) -ldl -lpthread -lm -lutil -lwoklib


#kentmodule_LTLIBRARIES = \
#	libwoklibmodule.la
	
#libwoklibmodule_la_SOURCE = $(libwokpython_la_SOURCES)
#libwoklibmodule_la_LDFLAGS = $(libwokpython_la_LDFLAGS)

SUBDIRS = \
	preloader 

## File created by the gnome-build tools


