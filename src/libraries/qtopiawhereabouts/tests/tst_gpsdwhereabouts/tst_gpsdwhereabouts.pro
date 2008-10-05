!qbuild:qtopia_project(unittest)
HEADERS=../qwhereaboutssubclasstest.h
SOURCES=tst_gpsdwhereabouts.cpp \
        ../qwhereaboutssubclasstest.cpp
!qbuild:depends(libraries/qtopiawhereabouts)

