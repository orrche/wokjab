libdir = $(prefix)/lib/woklib-plugins/secondary
moduledir = $(PY_EXEC_PREFIX)/lib/python$(PY_VER)/site-packages

AM_CXXFLAGS =\
	`python-config --cflags`
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


module_LTLIBRARIES = \
	Woklibmodule.la
	
Woklibmodule_la_SOURCES = $(libwokpython_la_SOURCES)
Woklibmodule_la_LDFLAGS = $(libwokpython_la_LDFLAGS) -module

SUBDIRS = \
	preloader 

## File created by the gnome-build tools


