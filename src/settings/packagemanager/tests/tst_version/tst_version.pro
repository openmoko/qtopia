!qbuild:qtopia_project( qtopia unittest )
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager
VPATH      +=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager
SOURCES+=tst_version.cpp \
         packageversion.cpp
HEADERS+=packageversion.h
!qbuild:depends(libraries/qtopia)
!qbuild:depends(libraries/qtopiabase)
