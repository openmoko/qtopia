!qbuild {
qtopia_project(qtopia app)

depends(examples/weblite/client)
depends(src/libraries/homeui)
depends(libraries/qtopiapim)
#CONFIG+=qtopia_main

# Only build for home edition
requires(equals(QTOPIA_UI,home))

TARGET=radioplayer

CONFIG+=no_tr no_singleexec no_sxe_test link_prl
LIBS += -L../../client -lwebliteclient -L../../feeds -lweblitefeeds
}
INCLUDEPATH+=../../client
INCLUDEPATH+=../../feeds
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim
SOURCES=radioplayer.cpp

pics.files=radio.png
pics.path=/pics/weblite/radioplayer
INSTALLS+=pics

# Package information (used for make packages)
pkg.name=radioplayer
pkg.desc=Radio Player Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

desktop.files=$$PWD/radioplayer.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
