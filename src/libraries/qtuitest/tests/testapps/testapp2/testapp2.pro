!qbuild {
    qtopia_project(qtopia app)
    depends(libraries/qtopiaphone)
}
TARGET=testapp2
CONFIG*=qtopia_main no_tr link_test
SOURCES*=main.cpp

desktop.files=testapp2.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pkg.domain=trusted

