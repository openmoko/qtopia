TEMPLATE=app
CONFIG+=qt
TARGET=content_installer

MODULES*=\
    qtopia::headers\
    qtopiabase::headers
QT*=sql

SOURCEPATH+=\
    /src/libraries/qtopiabase\
    /src/libraries/qtopia\
    /src/tools/dbmigrate

include(content_installer.pro)

