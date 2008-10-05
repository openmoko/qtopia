!qbuild:qtopia_project(unittest)

SOURCES=tst_qdawg.cpp
INSTALLS+=words
words.path=PATH
!qbuild:depends(libraries/qtopia)
