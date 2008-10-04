!qbuild {
qtopia_project(qtopia core lib)

# Only build for home edition
requires(equals(QTOPIA_UI,home))

TARGET=webliteclient
CONFIG+=no_singleexec create_prl
}

HEADERS=\
    webliteclient.h \
    webliteimg.h
    
SOURCES=\
    webliteclient.cpp \
    webliteimg.cpp

# Package information (used for make packages)
pkg.name=webliteclient
pkg.desc=WebLite Engine Client
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

