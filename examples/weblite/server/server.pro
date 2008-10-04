!qbuild {
qtopia_project(app qtopia)

# Only build for home edition
requires(equals(QTOPIA_UI,home))

TARGET=webliteserver
}
INCLUDEPATH+=../client

HEADERS=\
    webliteserver.h \
    
SOURCES=webliteserver.cpp

# Package information (used for make packages)
pkg.name=webliteserver
pkg.desc=WebLite Engine
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

settings.files = weblite-server.conf
settings.path = /etc/default/Trolltech

INSTALLS+=settings
