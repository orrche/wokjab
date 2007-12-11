libdir = $(prefix)/lib/woklib-plugins/normal

AM_CXXFLAGS = -lwoklib `pkg-config glib-2.0 sqlite3 --cflags`

lib_LTLIBRARIES = \
	libhistorysqlite.la

libhistorysqlite_la_SOURCES = \
	main.cc              \
	history.hpp              \
	history.cpp

libhistorysqlite_la_LDFLAGS = \
	`pkg-config glib-2.0 sqlite3 --libs`
## File created by the gnome-build tools

