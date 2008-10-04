!qbuild {
    qtopia_project(qtopia plugin)
    TARGET=smilviewer
    requires(enable_cell)

    depends(libraries/qtopiamail)
    depends(libraries/qtopiasmil)
}

HEADERS+=\
    smilviewer.h

SOURCES+=\
    smilviewer.cpp

