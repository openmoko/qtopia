!qbuild:qtopia_project(unittest)

!qbuild:VPATH+=$$PROJECT_ROOT/src/libraries/mylib
!qbuild:INCLUDEPATH+=$$PROJECT_ROOT/src/libraries/mylib
HEADERS+=mylib.h
SOURCES+=mylib.cpp
SOURCES+=main.cpp


