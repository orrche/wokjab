AM_CXXFLAGS = -lwoklib pkg-config openssl --cflags

libdir = $(prefix)/lib/woklib-plugins/normal

lib_LTLIBRARIES = \
	libjabberconnection.la

libjabberconnection_la_SOURCES = \
	main.cc       \
	Jabber.cc       \
	Jabber.h       \
	Connection.cc       \
	Connection.h       \
	IQ_auth.cc       \
	IQauth.h       \
	SSL.cc       \
	SSL.h       \
	XML_Input.cc       \
	XML_Input.h       \
	XML_Output.cc       \
	XML_Output.h       \
	staticfunctions.cc       \
	staticfunctions.h       \
	IQAuthManager.cc       \
	IQAuthManager.h

libjabberconnection_la_LDFLAGS = \
	pkg-config openssl --libs
	
## File created by the gnome-build tools


