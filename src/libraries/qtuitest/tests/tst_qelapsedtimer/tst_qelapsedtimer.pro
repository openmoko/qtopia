!qbuild:qtopia_project(unittest)

!qbuild:VPATH       += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
!qbuild:INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest

SOURCES+= \
    tst_qelapsedtimer.cpp \
    qelapsedtimer.cpp

HEADERS+= \
    qelapsedtimer_p.h

