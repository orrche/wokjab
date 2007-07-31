libdir = $(prefix)/lib/woklib-plugins/normal

AM_CFLAGS =\
	`pkg-config gtk+-2.0 --cflags`\
	 -I.. \
	 -Wall

AM_CXXFLAGS =\
	`pkg-config gtk+-2.0 --cflags`\
	 -Wall -lwoklib
	 
	 
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

libgtkpreference_la_LDFLAGS = \
	`pkg-config libglade-2.0 gtk+-2.0 --libs`
	
## File created by the gnome-build tools


