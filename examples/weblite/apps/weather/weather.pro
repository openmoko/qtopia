!qbuild {
qtopia_project(qtopia app)

depends(examples/weblite/client)
depends(examples/weblite/feeds)
depends(src/libraries/homeui)
depends(libraries/qtopiapim)

# Only build for home edition
requires(equals(QTOPIA_UI,home))

TARGET=webweather
CONFIG+=no_tr no_singleexec no_sxe_test link_prl
LIBS += -L../../client -lwebliteclient -L../../feeds -lweblitefeeds
}
INCLUDEPATH+=../../client
INCLUDEPATH+=../../feeds
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim
SOURCES=weather.cpp

pics.files=pics/*.png
pics.path=/pics/weblite/weather
INSTALLS+=pics

# Package information (used for make packages)
pkg.name=weather 
pkg.desc=Yahoo Weather Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

desktop.files=$$PWD/weather.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

settings.files=$$PWD/weather.conf
settings.path = /etc/default/Trolltech
INSTALLS+=settings
