!qbuild {
qtopia_project(app)
TARGET=radio_demux
#CONFIG-=qtopia
#LIBS-=-lqtopia-sqlite
#LIBS+=-L../../../../lib
# Only build for home edition
requires(equals(QTOPIA_UI,home))

}
SOURCES=radio_demux.cpp
# Package information (used for make packages)
pkg.name=radiodemux
pkg.desc=Radio Demultiplexer
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

