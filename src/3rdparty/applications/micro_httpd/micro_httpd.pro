qtopia_project(desktop app)

CONFIG-=qt
CONFIG+=no_tr

SOURCES=micro_httpd.c

TARGET=micro_httpd

LIBS=-lc

QMAKE_LIBS_DYNLOAD=

QMAKE_CFLAGS_RELEASE=-Os
QMAKE_LFLAGS_RELEASE=-nodefaultlibs
