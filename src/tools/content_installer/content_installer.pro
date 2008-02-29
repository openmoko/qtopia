qtopia_project(desktop app)
TARGET=content_installer
CONFIG+=no_tr

RESOURCES+=qtopia.qrc
# for 4.3, link in the qrc and use QResource to iterate over the tables
# RESOURCES+=qtopiapim.qrc

### Needs routines from qtopia
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopia
INCLUDEPATH+=$$QPEDIR/include/qtopia

### Also needs qLog
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase
INCLUDEPATH+=$$QPEDIR/include/qtopiabase

### Also needs the QtopiaPIM database stuff
VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim
INCLUDEPATH+=$$QPEDIR/include/qtopiapim
RESOURCES+=qtopiapim.qrc

DEFINES+=QTOPIA_CONTENT_INSTALLER

SOURCES	= main.cpp \
    qtopiasql.cpp \
    qtopialog.cpp \
    qlog.cpp \
    contentlnksql_p.cpp \
    qcontentfilter.cpp \
    qcontent.cpp \
    qthumbnail.cpp \
    thumbnailview_p.cpp \
    qcontentset.cpp \
    qdrmcontent.cpp \
    qdrmrights.cpp \
    drmcontent_p.cpp \
    qcontentplugin.cpp \
    contentlnk_p.cpp \
    contentpluginmanager_p.cpp \
    qcategorymanager.cpp \
    qtopiaipcadaptor.cpp \
    qtopiaipcenvelope.cpp \
    qtopiachannel.cpp \
    qtopianamespace.cpp \
    qpluginmanager.cpp \
    qstorage.cpp \
    qmimetype.cpp \
    qtopiaservices.cpp \
    qslotinvoker.cpp \
    qsignalintercepter.cpp \
    quniqueid.cpp 

HEADERS = \
    qtopiasql.h \
    qtopialog.h \
    qlog.h \
    contentlnksql_p.h \
    qcontentfilter.h \
    qcontent.h \
    qcontentset.h \
    thumbnailview_p.h \
    qdrmcontent.h \
    qdrmrights.h \
    drmcontent_p.h \
    qcontentplugin.h \
    contentlnk_p.h \
    qthumbnail.h \
    contentpluginmanager_p.h \
    qcategorymanager.h \
    qtopiaipcadaptor.h \
    qtopiaipcenvelope.h \
    qtopiachannel.h \
    qtopianamespace.h \
    qpluginmanager.h \
    qstorage.h \
    qmimetype.h \
    qtopiaservices.h \
    qslotinvoker.h \
    qsignalintercepter.h \
    quniqueid.h
