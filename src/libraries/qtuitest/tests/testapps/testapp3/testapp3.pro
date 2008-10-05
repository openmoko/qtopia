!qbuild {
    qtopia_project(qtopia app)
    depends(libraries/qtopiaphone)
}
TARGET=testapp3
CONFIG*=qtopia_main no_tr link_test
SOURCES*=main.cpp

desktop.files=testapp3.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pkg.domain=trusted

