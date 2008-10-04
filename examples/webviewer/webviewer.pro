!qbuild {
qtopia_project(qtopia app) # see buildsystem.html for more project keywords
TARGET=webviewer
# main.cpp uses the QTOPIA_ADD_APPLICATION/QTOPIA_MAIN macros
# It can also build without these macros if you disable this and change the define in main.cpp
CONFIG+=qtopia_main

# Do not build this app into a singleexec binary
CONFIG+=no_singleexec

# Messaging library is not available in platform edition
requires(!platform)

depends(libraries/qtopiaphone)
depends(libraries/qtopiamail)
requires(contains(QTE_CONFIG,webkit))
}
QT+=webkit

# Specify the languages that make lupdate should produce .ts files for
AVAILABLE_LANGUAGES=en_US
# Specify the langauges we want to install translations for
LANGUAGES=$$AVAILABLE_LANGUAGES

# These are the source files that get built to create the application
FORMS=cookies.ui cookiesexceptions.ui
HEADERS=webviewer.h navigationbar.h bindings.h cookiejar.h util.h softnavigationbar.h plugins/s60wrt/systeminfo.h
SOURCES=main.cpp webviewer.cpp navigationbar.cpp bindings.cpp cookiejar.cpp util.cpp softnavigationbar.cpp plugins/s60wrt/systeminfo.cpp

# Install the launcher item. The metadata comes from the .desktop file
# and the path here.
desktop.files=webviewer.desktop weblets/*.desktop
desktop.path=/apps/Applications
desktop.trtarget=webviewer-nct
desktop.hint=nct desktop
INSTALLS+=desktop

# Install service registration
service.files=$$PWD/services/WebAccess/webviewer
service.path=/services/WebAccess
INSTALLS+=service

webaccess.files=$$device_overrides(/etc/default/Trolltech/WebAccess.conf)
webaccess.path=/etc/default/Trolltech
INSTALLS+=webaccess

# Install HTML
web.files=web/*.html
web.path=/etc/web
INSTALLS+=web

# Install homescreen html
!qbuild {
homescreen.path=/etc/web
homescreen.commands=cp -Rf $$PWD/weblets/homescreen $(INSTALL_ROOT)$$homescreen.path
INSTALLS+=homescreen
}

# Install canned homescreen XML files
homescreencanned.path=/weblets/canned
homescreencanned.files=weblets/homescreen/canned/*.xml
INSTALLS+=homescreencanned

# Install some pictures.
pics.files=pics/*
pics.path=/pics/webviewer
pics.hint=pics
INSTALLS+=pics

# Install help files
help.source=help
help.files=webviewer.html
help.hint=help
INSTALLS+=help

# Package information (used for make packages)
pkg.name=webviewer
pkg.desc=WebViewer Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=Commercial
pkg.domain=trusted
