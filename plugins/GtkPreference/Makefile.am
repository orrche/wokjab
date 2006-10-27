libdir = $(prefix)/lib/woklib-plugins/normal

AM_CFLAGS =\
	`pkg-config gtk+-2.0 --cflags`\
	 -I.. \
	 -Wall

AM_CXXFLAGS =\
	`pkg-config gtk+-2.0 --cflags`\
	 -I..\
	 -I../../../lib/woklib/include\
	 -Wall
	 
	 
lib_LTLIBRARIES = \
	libgtkpreference.la 

libgtkpreference_la_SOURCES = \
	main.cc              \
	GtkPreference.cc              \
	GtkPreference.h              \
	GtkPString.h              \
	GtkPString.cc              \
	GtkPText.h              \
	GtkPText.cc              \
	GtkPBool.h              \
	GtkPBool.cc              \
	GtkPCommon.cc              \
	GtkPCommon.h              \
	GtkPPassword.h              \
	GtkPPassword.cc 

## File created by the gnome-build tools


