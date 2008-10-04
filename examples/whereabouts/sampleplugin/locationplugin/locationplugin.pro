!qbuild{
    qtopia_project(qtopia plugin)
    TARGET=locationplugin
    CONFIG+=no_tr
    depends(libraries/qtopiawhereabouts)
    requires(enable_qtopiawhereabouts)
}

HEADERS = locationplugin.h
SOURCES = locationplugin.cpp
