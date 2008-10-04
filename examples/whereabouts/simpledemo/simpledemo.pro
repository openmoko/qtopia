!qbuild{
    qtopia_project(qtopia app)
    TARGET=simplelocationdemo
    CONFIG+=qtopia_main no_quicklaunch no_tr
    depends(libraries/qtopiawhereabouts)
    requires(enable_qtopiawhereabouts)
}

HEADERS=
SOURCES=main.cpp
