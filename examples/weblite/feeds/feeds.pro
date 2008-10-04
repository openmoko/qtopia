!qbuild{
qtopia_project(qtopia lib)
depends(examples/weblite/client)
# Only build for home edition
requires(equals(QTOPIA_UI,home))
TARGET=weblitefeeds
CONFIG+=no_singleexec
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/examples/weblite/client
}

HEADERS=\
    webfeed.h \
    mediafeed.h \
    
SOURCES= \
    webfeed.cpp \
    mediafeed.cpp \

# Package information (used for make packages)
pkg.name=weblitefeed
pkg.desc=WebLite Feed Parser
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

#settings.files = weblite-server.conf
#settings.path = /etc/default/Trolltech

#INSTALLS+=settings
