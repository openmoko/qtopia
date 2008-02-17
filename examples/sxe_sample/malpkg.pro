qtopia_project(qtopia app)
TARGET=malpkg

# Input
HEADERS += malpkg.h \

SOURCES += main.cpp \
    malpkg.cpp 

# IMAGES +=images/qtlogo-small.png

unix:LIBS += -lcrypt

desktop.files=malpkg.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
desktop.trtarget=malpkg-nct
INSTALLS+=desktop

pics.files=mal.png
pics.path=/pics/malpkg
INSTALLS+=pics

pkg.name=malpkg
pkg.desc=Sample Malware Application
pkg.domain=trusted
pkg.version=1.0.0
pkg.multi=pkgA pkgB pkgC pkgD pkgE pkgF pkgG pkgH pkgJ pkgK
pkg.maintainer=Sarah Smith <sarah.smith@trolltech.com>
pkg.license=Commercial

help.files=malpkg.html
help.hint=help
INSTALLS+=help
