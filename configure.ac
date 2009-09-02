#
#  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
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


AC_INIT([wokjab], [0.9.14], [nedo80@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AM_CONFIG_HEADER([config.h])
AC_PROG_INSTALL
AC_PROG_LIBTOOL
#AC_PROG_RANLIB
AM_PATH_GTK_2_0
AM_PROG_LIBTOOL
AM_GNU_GETTEXT_VERSION([0.14])
AM_GNU_GETTEXT([external])

dnl Set PACKAGE PREFIX
if test "x${prefix}" = "xNONE"; then
  prefix=${ac_default_prefix}
#else
#  prefix=${prefix}
fi

AC_ARG_ENABLE(python,
[  --disable-python        don't build the python plugin],
        python=$enableval, python=yes)



dnl *********************************************************************
dnl ** PYTHON ***********************************************************
dnl *********************************************************************

if test "$python" = yes; then
        AC_PATH_PROG(pythonpath, python2)
        if test "_$pythonpath" = _ ; then
                AC_PATH_PROG(pythonpath, python)
        fi
        if test "_$pythonpath" = _ ; then
                python=no
        else
                AC_MSG_CHECKING(Python version)
                changequote(<<, >>)dnl
                PY_VER=`$pythonpath -c 'import distutils.sysconfig; print distutils.sysconfig.get_config_vars("VERSION")[0];'`
                PY_LIB=`$pythonpath -c 'import distutils.sysconfig; print distutils.sysconfig.get_python_lib(standard_lib=1);'`
                PY_INC=`$pythonpath -c 'import distutils.sysconfig; print distutils.sysconfig.get_config_vars("INCLUDEPY")[0];'`
                $pythonpath -c "import sys; map(int,sys.version[:3].split('.')) >= [2,2] or sys.exit(1)"
                changequote([, ])dnl
                AC_MSG_RESULT($PY_VER)
                if test "$?" != "1"; then
                        AC_MSG_CHECKING(Python compile flags)
                        PY_PREFIX=`$pythonpath -c 'import sys; print sys.prefix'`
                        PY_EXEC_PREFIX=`$pythonpath -c 'import sys; print sys.exec_prefix'`
                        if test -f $PY_INC/Python.h; then
                                PY_LIBS="-L$PY_LIB/config -lpython$PY_VER -lpthread -lutil"
                                PY_CFLAGS="-I$PY_INC"
                                AC_MSG_RESULT(ok)
                        else
                                python=no
                                AC_MSG_RESULT([Can't find Python.h])
                        fi
                else
                        echo "Python too old. Only 2.2 or above is supported."
                        python=no
                fi
        fi
fi

dnl make these visible to all Makefiles
AC_SUBST(PY_CFLAGS)
AC_SUBST(PY_LIBS)
AC_SUBST(PY_VER)
AC_SUBST(PY_EXEC_PREFIX)

AM_CONDITIONAL(DO_PYTHON, test "x$python" = "xyes")

AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], ["${prefix}/share"], ["Due to growing insanity I dont do this the proper way I think, Anyways the pixmap dir"])
AC_DEFINE_UNQUOTED([PACKAGE_PLUGIN_DIR], ["${prefix}/lib/woklib-plugins"], ["Where the system plugins gets stored"])
AC_DEFINE_UNQUOTED([PACKAGE_GLADE_DIR], ["${prefix}/lib/woklib/glade"], ["Where the system UI gets stored"])


AC_CONFIG_FILES(Makefile \
	po/Makefile.in \
	plugins/Makefile  \
	plugins/normal/Makefile \
	plugins/normal/GnomeStatusIcon/Makefile  \
	plugins/normal/GnomeStatusIcon/src/Makefile  \
	plugins/normal/GnomeStatusIcon/include/Makefile  \
        plugins/normal/GUIMessageHandler/Makefile  \
        plugins/normal/GUIWindowDock/Makefile  \
        plugins/normal/GUIWindow/Makefile  \
        plugins/normal/GUIStatusViewer/Makefile  \
        plugins/normal/GUIPresentReqWidget/Makefile  \
        plugins/normal/GUIPluginWindow/Makefile  \
	plugins/normal/Jabber/Makefile \
        plugins/normal/JabberConnection/Makefile  \
        plugins/normal/JabberRegistration/Makefile  \
        plugins/normal/JIDConfig/Makefile  \
	plugins/normal/Tooltip/Makefile  \
	plugins/system/Makefile \
	plugins/experimental/Makefile  \
        plugins/experimental/Dice/Makefile  \
        plugins/experimental/P2PRandomNumberMonitor/Makefile  \
        plugins/experimental/AtomNotifications/Makefile  \
        plugins/experimental/xdataConfig/Makefile  \
	plugins/MenuResource/Makefile  \
	plugins/MenuRoster/Makefile  \
	plugins/GtkTimer/Makefile  \
	plugins/GtkTimer/src/Makefile  \
	plugins/GtkPreference/Makefile  \
	plugins/GtkMenu/Makefile  \
        plugins/Gtk-jep0060-PublishSubscribe/Makefile  \
        plugins/GUIRoster/Makefile  \
        plugins/GroupChatBookmark/Makefile  \
        plugins/GroupManager/Makefile  \
        plugins/GPGenc/Makefile  \
	plugins/OpenDialogTo/Makefile  \
	plugins/wokjab-jid-info/Makefile  \
	plugins/wokjab-jid-info/src/Makefile  \
	plugins/wokjab-jid-info/include/Makefile  \
	plugins/roster-rename/Makefile  \
	plugins/roster-rename/src/Makefile  \
	plugins/guibrowser/Makefile  \
	plugins/guibrowser/src/Makefile  \
	plugins/registerservice/Makefile  \
	plugins/registerservice/src/Makefile  \
        plugins/rfc1928-SOCKS5/Makefile  \
	plugins/DisplayGtk/Makefile  \
	plugins/DisplayGtk/src/Makefile  \
	plugins/Config/Makefile  \
        plugins/CrashManager/Makefile  \
	plugins/Authorization/Makefile  \
	plugins/AutoPresence/Makefile  \
        plugins/AutoJoinGroupChat/Makefile  \
	plugins/woklib-groupchat/Makefile  \
	plugins/woklib-groupchat/src/Makefile  \
	plugins/groupchat-plugin/Makefile  \
	plugins/groupchat-plugin/src/Makefile  \
	plugins/groupchat-plugin/glade/Makefile  \
	plugins/groupchat-plugin/images/Makefile  \
	plugins/Reversi/Makefile  \
	plugins/Reversi/pixmaps/Makefile  \
        plugins/RosterEvent/Makefile  \
	plugins/EventSound/Makefile  \
        plugins/EventManager/Makefile  \
	plugins/toaster/Makefile  \
	plugins/python/Makefile  \
        plugins/python/preloader/Makefile  \
	plugins/ScriptLoader/Makefile  \
        plugins/JabberRoster/Makefile  \
	plugins/jep0004-xdata/Makefile  \
	plugins/jep0004-xdata/src/Makefile  \
	plugins/jep0004-xdata/include/Makefile  \
	plugins/jep0030-Disco/Makefile  \
        plugins/jep0050-AdHocCommands/Makefile  \
	plugins/jep0065-Bytestream/Makefile \
	plugins/jep0065-Bytestream/src/Makefile  \
        plugins/jep0060-PublishSubscribe/Makefile  \
	plugins/jep0096-Filetransfear/Makefile \
	plugins/jep0096-Filetransfear/src/Makefile  \
        plugins/jep0096-Filetransfear/pixmap/Makefile  \
	plugins/jep0115-EntCap/Makefile  \
        plugins/jep0118-UserTune/Makefile  \
        plugins/jep0153-VCardAvatar/Makefile  \
	plugins/jep0163-PEP/Makefile \
	plugins/P2PRandomNumber/Makefile  \
        plugins/Idle/Makefile  \
        plugins/Fileshare/Makefile  \
        plugins/dbus/Makefile  \
        plugins/History.SQLite/Makefile  \
        plugins/HtmlLinkMessage/Makefile  \
        plugins/normal/WokjabDockWindow/Makefile  \
        plugins/normal/NotificationWidget/Makefile  \
        plugins/normal/XEP-0147-XMPP_URI_Scheme_Query/Makefile  \
        plugins/normal/AddJIDWidget/Makefile  \
        plugins/normal/ConnectWindow/Makefile  \
        plugins/normal/Roster/Makefile  \
	wokjab/Makefile \
	wokjab/src/Makefile \
	wokjab/include/Makefile \
	wokjab/pixmaps/Makefile \
	wokjab/glade/Makefile \
)

AC_OUTPUT
