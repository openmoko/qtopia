CONFIG += qtopiaplugin

TARGET=fraction

HEADERS = fraction.h  fractionfactory.h
SOURCES = fraction.cpp fractionfactory.cpp

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/calculator/fraction.desktop
desktop.path=/plugins/calculator/
INSTALLS+=desktop

TRANSLATABLES = $${HEADERS} $${SOURCES}
LIBS   +=-lqtopiacalc
