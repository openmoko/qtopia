# Note: this requires using the $QPEDIR/bin/qtopiamake script

qtopia_project(qtopia app)

# TEMPLATE = app
# INCLUDEPATH += .

# Disable i18n support
CONFIG+=no_tr

# Input
HEADERS += trust.h \

SOURCES += main.cpp \
    trust.cpp 

# DEFINES += Q_WS_QWS

# IMAGES +=images/qtlogo-small.png

# LIBS += -lqtopia
unix {
LIBS += -lcrypt
}

TARGET = trust

TARGET.path = /bin

desktop.files = trust.desktop
desktop.path = /apps/Applications
INSTALLS += desktop

pics.files=trust.png
pics.path=/pics/trust
INSTALLS+=pics

cert.files=trustcorp.pem
cert.path=/etc/Trust
cert.hint=cert
INSTALLS+=cert

pkg.name=trust
pkg.desc=Sample Trusted Application
pkg.domain=trusted
pkg.version=1.0.0
pkg.maintainer=Sarah Smith <sarah.smith@trolltech.com>
pkg.trust=/etc/Trust/trustcorp.pem
pkg.license=Commercial

help.files=trust.html
help.path=/help/html
INSTALLS+=help

TRANSLATABLES = $$HEADERS $$SOURCES
NON_CODE_TRANSLATABLES = $$desktop.files
NON_CODE_TRTARGETS = trust-nct
