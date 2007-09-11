AM_CXXFLAGS = -lwoklib `pkg-config glib-2.0 --cflags` 

pkgdata_DATA = \
	group.png

EXTRA_DIST = ${pkgdata_DATA}

libdir = $(prefix)/lib/woklib-plugins/normal

lib_LTLIBRARIES = \
	libjabberroster.la 

libjabberroster_la_SOURCES = \
	main.cc             \
	JabberRoster.cc             \
	JabberRoster.h             \
	JabberSession.h             \
	JabberSession.cc             \
	User.cc             \
	User.h             \
	Group.cc             \
	Group.h 

libjabberroster_la_LDFLAGS = \
	`pkg-config glib-2.0 --libs` -lwoklib


## File created by the gnome-build tools


