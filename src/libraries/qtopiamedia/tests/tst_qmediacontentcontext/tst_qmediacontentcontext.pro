!qbuild:qtopia_project(unittest)
SOURCES+=tst_qmediacontentcontext.cpp
#VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiamedia
#SOURCES+=qmediatools.cpp
#HEADERS+=qmediatools.h
!qbuild:depends(libraries/qtopiamedia)

