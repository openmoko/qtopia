qtopia_project(desktop external app)
include(../../libraries/sqlite/sqlite.pro)
CONFIG-=syncqtopia
INSTALLS-=sdk_headers
TARGET=sqlite
VPATH+=../../libraries/sqlite

SOURCES+=\
    shell.c
LIBS+=-lpthread

