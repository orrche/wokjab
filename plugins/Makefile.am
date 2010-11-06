#
#  Copyright (C) 2003-2010  Kent Gustavsson <nedo80@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

if DO_PYTHON
pythondir = python
endif

if DO_GTK
gtkdir = \
	DisplayGtk \
	Fileshare \
	GtkTimer \
	GtkPreference \
	GtkMenu \
	guibrowser \
	GUIRoster \
	OpenDialogTo \
	Reversi \
	Fileshare \
	RosterEvent \
	groupchat-plugin \
	jep0004-xdata \
	jep0050-AdHocCommands \
	jep0065-Bytestream \
	jep0096-Filetransfear \
	jep0153-VCardAvatar \
	registerservice \
	roster-rename \
	ScriptLoader \
	toaster \
	wokjab-jid-info \
	AutoJoinGroupChat \
	CrashManager \
	Gtk-jep0060-PublishSubscribe \
	jep0163-PEP \
	GroupManager 
endif


SUBDIRS= \
	normal \
	system \
	experimental \
	Authorization \
	AutoPresence \
	Config \
	EventSound \
	JabberRoster \
	jep0030-Disco \
	jep0050-AdHocCommands-Manager \
	jep0115-EntCap \
	MenuResource \
	MenuRoster \
	woklib-groupchat \
	rfc1928-SOCKS5 \
	Idle \
	GroupChatBookmark \
	dbus \
	History.SQLite \
	jep0118-UserTune \
	jep0060-PublishSubscribe \
	EventManager \
	HtmlLinkMessage \
	GPGenc \
	P2PRandomNumber \
	$(gtkdir) \
	$(pythondir) 

