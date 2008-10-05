!qbuild:qtopia_project(unittest)

DEFINES+=SXE_INSTALLER

SOURCES=tst_qpackageregistry_SXE_INSTALLER.cpp

!qbuild:VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiasecurity
!qbuild:INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiasecurity
!qbuild:VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase
!qbuild:INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase

SOURCES+=qpackageregistry.cpp qsxepolicy.cpp keyfiler.cpp qtopiasxe.cpp
HEADERS+=qpackageregistry.h keyfiler_p.h qsxepolicy.h qtopiasxe.h

!qbuild:depends(libraries/qtopia)
