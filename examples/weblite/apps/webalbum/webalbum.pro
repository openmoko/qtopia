!qbuild {
qtopia_project(qtopia app)
depends(examples/weblite/client)
depends(examples/weblite/feeds)
depends(src/libraries/homeui)
depends(libraries/qtopiapim)
TARGET=webalbum

CONFIG+=no_tr no_singleexec no_sxe_test link_prl
LIBS += -L../../client -lwebliteclient -L../../feeds -lweblitefeeds
#CONFIG+=qtopia_main
}
requires(equals(QTOPIA_UI,home))

INCLUDEPATH+=../../client
INCLUDEPATH+=../../feeds
INCLUDEPATH+=../../../../src/libraries/qtopiapim
SOURCES=webalbum.cpp

# Package information (used for make packages)
pkg.name=webalbum
pkg.desc=Web Album Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted
pics.files=$$PWD/webalbum.png
pics.path=/pics/weblite/album
INSTALLS+=pics
desktop.files=$$PWD/webalbum.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
settings.files = webalbum.conf
settings.path = /etc/default/Trolltech
INSTALLS+=settings
