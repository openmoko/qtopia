!qbuild:qtopia_project(unittest)
SOURCES+=tst_qdsaction.cpp
!qbuild:VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiabase
SOURCES+=qtopiachannel.cpp
!qbuild:depends(libraries/qtopia)
