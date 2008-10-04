TEMPLATE=lib
CONFIG+=embedded singleexec
TARGET=qtopia-sqlite

MODULE_NAME=sqlite
LICENSE=FREEWARE

# Technically, this depends on Qt but that would create a circular dependency
# All this needs is to include qconfig.h so just set INCLUDEPATH
# FIXME this should open the Qt project (but that could lead to a race condition)
INCLUDEPATH+=$$QTEDIR/include/QtCore

release:DEFINES*=NDEBUG

SOURCEPATH=/qtopiacore/qt/src/3rdparty/sqlite

include(sqlite.pro)
