#
# $Id: Makefile.am,v 1.4 2002/12/02 11:22:55 telka Exp $
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

noinst_LIBRARIES = libkconfig.a
if QCONF
bin_PROGRAMS = conf mconf qconf
else
bin_PROGRAMS = conf mconf
endif

nodist_libkconfig_a_SOURCES = zconf.tab.c lex.zconf.c zconf.tab.h

conf_SOURCES = conf.c lkc.h lkc_proto.h
conf_LDADD = -L. -lkconfig

mconf_SOURCES = mconf.c lkc.h lkc_proto.h
mconf_LDADD = -L. -lkconfig

if QCONF
qconf_SOURCES = qconf.cc lkc.h images.c lkc_proto.h qconf.h
nodist_qconf_SOURCES = qconf.moc
qconf_LDADD = -L. -lkconfig -L@QTDIR@/lib -lqt
qconf_CXXFLAGS = -I@QTDIR@/include -DLKC_DIRECT_LINK
endif

if QCONF
BUILT_SOURCES = zconf.tab.c lex.zconf.c zconf.tab.h qconf.moc
else
BUILT_SOURCES = zconf.tab.c lex.zconf.c zconf.tab.h
endif

zconf.tab.c: zconf.y
zconf.tab.h: zconf.tab.c

zconf.y: lex.zconf.c confdata.c symbol.c expr.c menu.c expr.h

if QCONF
qconf.moc: qconf.h
	@QTDIR@/bin/moc -i $< -o $@
endif

%.tab.c: %.y
	bison -t -d -v -b $* -p $(notdir $*) $<

lex.%.c: %.l
	flex -P$(notdir $*) -o$@ $<

EXTRA_DIST = zconf.y confdata.c symbol.c expr.c menu.c expr.h zconf.l

CLEANFILES = zconf.tab.c lex.zconf.c zconf.tab.h qconf.moc zconf.output lex.backup
