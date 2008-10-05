!qbuild:qtopia_project(qtopia app)
TARGET=testapp1
CONFIG*=qtopia_main no_tr link_test
SOURCES*=main.cpp

desktop.files=testapp1.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pkg.domain=trusted

