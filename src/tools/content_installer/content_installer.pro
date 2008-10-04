!qbuild{
qtopia_project(desktop app)
TARGET=content_installer
CONFIG+=no_tr

VPATH+=\
    $$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase\
    $$QTOPIA_DEPOT_PATH/src/libraries/qtopia\
    $$QTOPIA_DEPOT_PATH/src/tools/dbmigrate
INCLUDEPATH+=\
    $$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase\
    $$QTOPIA_DEPOT_PATH/src/libraries/qtopia\
    $$QTOPIA_DEPOT_PATH/src/tools/dbmigrate
qt_inc(qtopia)
depends(libraries/qtopia,fake)
qt_inc(qtopiabase)
depends(libraries/qtopiabase,fake)
#qt_inc(qtopiapim)
#depends(libraries/qtopiapim,fake)
}

DEFINES+=QTOPIA_CONTENT_INSTALLER 
DEFINES+=QTOPIA_HOST 

# Semaphores might be required if local TCP ports cannot be used
# but they require a 2.6 kernel and GLibC 2.5 or later.
#CONFIG+=use_semaphores

RESOURCES=\
    qtopia.qrc

SOURCES=\
    main.cpp\

HEADERS=\
    qsystemsemaphore.h

use_semaphores {
    SOURCES+=qsystemsemaphore_sem.cpp
    # because we can't guarantee cleanup by glibc's termination routines.
    DEFINES+=USE_EVIL_SIGNAL_HANDLERS
} else {
    SOURCES+=qsystemsemaphore_tcp.cpp
    QT*=network
}

SOURCES+=\
    qtopiasql.cpp \
    qtopiasql_p.cpp \
    qtopialog.cpp \
    qlog.cpp \
    qcontentfilter.cpp \
    qcontent.cpp \
    qthumbnail.cpp \
    thumbnailview_p.cpp \
    qcontentset.cpp \
    qdrmcontent.cpp \
    qdrmrights.cpp \
    drmcontent_p.cpp \
    qcontentplugin.cpp \
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
    quniqueid.cpp \
    migrateengine.cpp \
    qcategorystore.cpp \
    qsqlcategorystore.cpp \
    qcontentengine.cpp \
    qfscontentengine.cpp \
    qcontentstore.cpp \
    qsqlcontentstore.cpp \
    qcontentsetengine.cpp \
    qmimetypedata.cpp \
    qcontentsortcriteria.cpp \
    qsqlcontentsetengine.cpp

HEADERS = \
    qtopiasql.h \
    qtopiasql_p.h \
    qtopialog.h \
    qlog.h \
    qcontentfilter.h \
    qcontent.h \
    qcontentset.h \
    thumbnailview_p.h \
    qdrmcontent.h \
    qdrmrights.h \
    drmcontent_p.h \
    qcontentplugin.h \
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
    quniqueid.h \
    migrateengine.h \
    qcategorystore_p.h \
    qsqlcategorystore_p.h \
    qcontentstore_p.h \
    qsqlcontentstore_p.h \
    qcontentsetengine_p.h \
    qmimetypedata_p.h \
    qsqlcontentsetengine_p.h

