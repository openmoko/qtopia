!qbuild {
qtopia_project(qtopia app)
TARGET=radiosvc
depends(libraries/qtopiamedia)
depends(examples/weblite/client)
CONFIG+=no_tr no_singleexec no_sxe_test link_prl
# Only build for home edition
requires(equals(QTOPIA_UI,home))

}
LIBS += -L../../client -lwebliteclient
INCLUDEPATH+= \
                ../../client \
                src/libraries/qtopiamedia
                
HEADERS=../radio_codes.h radio_service.h
SOURCES=radio_service.cpp

# Package information (used for make packages)
pkg.name=radiosvc
pkg.desc=WebLite Engine
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

# Install service registration
service.files=radiosvc
service.path=/services/WebRadio
INSTALLS+=service
