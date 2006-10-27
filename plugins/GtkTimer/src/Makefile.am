libdir = $(prefix)/lib/woklib-plugins/system

AM_CXXFLAGS =\
	`pkg-config gtk+-2.0 --cflags` -I../../../../lib/woklib/include -g -O3 -Wall

lib_LTLIBRARIES = \
	libgkttimer.la 

libgkttimer_la_SOURCES = \
	GtkTimer.cc     \
	GtkTimer.h     \
	main.cc     \
	GtkTimerSession.h     \
	GtkTimerSession.cc 

libgkttimer_la_LDFLAGS = \
	`pkg-config gtk+-2.0 --libs`


## File created by the gnome-build tools


