!qbuild:qtopia_project(unittest)

!qbuild:VPATH       += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
!qbuild:INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest

SOURCES+=                       \
    tst_qtuitestnamespace.cpp \
    demowidgets.cpp             \
    qalternatestack_unix.cpp         \
    qelapsedtimer.cpp           \
    qeventwatcher.cpp           \
    qinputgenerator_qws.cpp     \
    qtuitestnamespace.cpp     \
    qtuitestrecorder.cpp      \
    qtuitestwidgets.cpp

HEADERS+=                       \
    demowidgets_p.h             \
    qalternatestack_p.h         \
    qelapsedtimer_p.h           \
    qeventwatcher_p.h           \
    qinputgenerator_p.h         \
    qtuitestnamespace.h       \
    qtuitestrecorder.h        \
    qtuitestwidgets_p.h

!qbuild:depends(libraries/qtopia)

