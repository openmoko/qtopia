!qbuild {
qtopia_project(qtopia app)

depends(examples/weblite/client)
depends(examples/weblite/feeds)
depends(src/libraries/homeui)
depends(libraries/qtopiapim)
depends(libraries/qtopiamedia)
#CONFIG+=qtopia_main

# Only build for home edition
requires(equals(QTOPIA_UI,home))

TARGET=podcastviewer

CONFIG+=no_tr no_singleexec no_sxe_test link_prl
LIBS += -L../../client -lwebliteclient -L../../feeds -lweblitefeeds
}
INCLUDEPATH+=../../client
INCLUDEPATH+=../../feeds
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim
SOURCES=podcastviewer.cpp

pics.files=podcastviewer.png
pics.path=/pics/weblite/podcastviewer
INSTALLS+=pics

# Package information (used for make packages)
pkg.name=podcastviewer
pkg.desc=Podcast Viewer
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

desktop.files=$$PWD/podcastviewer.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

settings.files=$$PWD/podcastviewer.conf 
settings.path = /etc/default/Trolltech
INSTALLS+=settings

service.files=$$PWD/podcasts.xml
service.path = /etc
service.hint=service
INSTALLS+=service
