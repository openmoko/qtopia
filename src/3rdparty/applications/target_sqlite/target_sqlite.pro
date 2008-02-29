qtopia_project(embedded external app)
include(../../libraries/sqlite/sqlite.pro)
CONFIG-=syncqtopia
INSTALLS-=sdk_headers
TARGET=sqlite
VPATH+=../../libraries/sqlite
CONFIG-=enable_singleexec # build as a standalone binary

SOURCES+=\
    shell.c
LIBS+=-lpthread

