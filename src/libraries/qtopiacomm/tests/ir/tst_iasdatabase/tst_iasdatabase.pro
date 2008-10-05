!qbuild:qtopia_project(unittest)
HEADERS=
SOURCES=tst_iasdatabase.cpp
TARGET=tst_iasdatabase
requires(enable_infrared)
!qbuild:depends(libraries/qtopiacomm/ir) 
