!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiawhereabouts
CONFIG+=no_tr
}

HEADERS=\
    qwhereabouts.h \
    qwhereaboutscoordinate.h \
    qwhereaboutsupdate.h \
    qwhereaboutsfactory.h \
    qwhereaboutsplugin.h \
    qnmeawhereabouts.h

PRIVATE_HEADERS=\
    gpsdwhereabouts_p.h \
    qnmeawhereabouts_p.h

SOURCES=\
    qwhereabouts.cpp \
    qwhereaboutscoordinate.cpp \
    qwhereaboutsupdate.cpp \
    qwhereaboutsfactory.cpp \
    qwhereaboutsplugin.cpp \
    gpsdwhereabouts.cpp \
    qnmeawhereabouts.cpp

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/whereabouts
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}

settings.files=\
    $$device_overrides(/etc/default/Trolltech/Whereabouts.conf)
settings.path=/etc/default/Trolltech
INSTALLS+=settings
