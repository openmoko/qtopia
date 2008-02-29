#
# $Id: configure.ac,v 1.3 2002/12/02 10:25:40 telka Exp $
#
# Copyright (C) 2002 ETC s.r.o.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# Written by Marcel Telka <marcel@telka.sk>, 2002.
#

AC_INIT(lkc,2.5.50)
AC_PREREQ(2.53)
AC_CONFIG_AUX_DIR(tools)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME,AC_PACKAGE_VERSION)

AC_CONFIG_FILES(Makefile)

AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_CXX

AC_ARG_WITH(qt,
	AC_HELP_STRING([--with-qt=PATH], [Path to the qt]),
	QTDIR="$with_val",
	QTDIR="/usr/lib/qt3")

AC_MSG_CHECKING(qt)
if test -f $QTDIR/bin/moc ; then
	AC_MSG_RESULT($QTDIR)
	AC_SUBST(QTDIR)
else
	AC_MSG_RESULT(not found)
	QTDIR=""
fi

AM_CONDITIONAL(QCONF, test ! -z "$QTDIR")

CFLAGS="$CFLAGS -Wall"

AC_OUTPUT
