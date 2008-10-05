!qbuild:qtopia_project(unittest)

!qbuild:VPATH       += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
!qbuild:INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest

SOURCES+= \
    tst_qtuitestwidgets.cpp \
    demowidgets.cpp \
    qalternatestack_unix.cpp \
    qeventwatcher.cpp \
    qinputgenerator_qws.cpp \
    qtuitestnamespace.cpp \
    qtuitestrecorder.cpp \
    qtuitestwidgets.cpp \
    qtuitestwidgetinterface.cpp

HEADERS+= \
    demowidgets_p.h \
    qalternatestack_p.h \
    qeventwatcher_p.h \
    qinputgenerator_p.h \
    qtuitestnamespace.h \
    qtuitestrecorder.h \
    qtuitestwidgets_p.h

DEFINES+=QT_STATICPLUGIN

!qbuild:depends(libraries/qtopia)
