!qbuild{
!qbuild:qtopia_project(unittest)

!qbuild:VPATH       += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
!qbuild:INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
}

SOURCES+= \
    tst_qalternatestack.cpp \
    qalternatestack_unix.cpp

HEADERS+= \
    qalternatestack_p.h

