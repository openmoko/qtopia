!qbuild{
    qtopia_project(qtopia app)
    TARGET=mylocationapp
    CONFIG+=qtopia_main no_tr
    depends(libraries/qtopiawhereabouts)
    requires(enable_qtopiawhereabouts)
}

HEADERS=mylocationapp.h
SOURCES=mylocationapp.cpp \
        main.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/examples/whereabouts/sampleplugin/mylocationapp/mylocationapp.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=mylocationapp
pkg.desc=My Location App
pkg.domain=trusted

# Install pictures.
pics.files=pics/*
pics.path=/pics/mylocationapp
pics.hint=pics
INSTALLS+=pics
