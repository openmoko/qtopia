!qbuild:qtopia_project(unittest)
HEADERS=
SOURCES=tst_localdevice.cpp
TARGET=tst_localdevice
requires(enable_infrared)
!qbuild:depends(libraries/qtopiacomm/ir) 
