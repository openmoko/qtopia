qtopia_project(qtopia app)
TARGET=dbservices
CONFIG+=qtopia_main no_quicklaunch

HEADERS = \
    dbservicesui.h \
    service.h \
    dbrestoreop.h \
    dboperation.h \
    schema.h
                  
SOURCES	= \
    dbservicesui.cpp \
    service.cpp \
    dbrestoreop.cpp \
    dboperation.cpp \
    schema.cpp \
    main.cpp

depends(libraries/qtopia)

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=dbservices*
help.hint=help

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/DbServices.desktop
desktop.path=/apps/Settings
desktop.hint=desktop

pics.files=$$QTOPIA_DEPOT_PATH/pics/dbservices/*
pics.path=/pics/dbservices
pics.hint=pics

service.files=$$QTOPIA_DEPOT_PATH/services/DbServices/dbservices
service.path=/services/DbServices

INSTALLS+=help pics service

pkg.desc=Database Services utility, including restore and migration
pkg.domain=window,docapi
