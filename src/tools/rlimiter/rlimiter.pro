!qbuild{
qtopia_project( external app )
CONFIG-=qt
TARGET=rlimiter
# Pull in only the C library
LIBS=-lc
QMAKE_LIBS_DYNLOAD=
QMAKE_CFLAGS_RELEASE=-Os
QMAKE_LFLAGS_RELEASE=-nodefaultlibs
}

SOURCES=main.c

