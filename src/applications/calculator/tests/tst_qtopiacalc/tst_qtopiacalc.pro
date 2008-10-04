!qbuild:qtopia_project(unittest)
### Needs routines from calculator
!qbuild:VPATH += $$QTOPIA_DEPOT_PATH/src/applications/calculator
!qbuild:INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/applications/calculator
!qbuild:depends(libraries/qtopia)
HEADERS += engine.h \
           instruction.h \
           doubleinstruction.h \
           doubledata.h \
           data.h
SOURCES += tst_qtopiacalc.cpp \
           engine.cpp \
           instruction.cpp \
           doubleinstruction.cpp \
           doubledata.cpp \
           data.cpp