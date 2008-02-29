CONFIG += qtopiaplugin

TARGET = simple

HEADERS = simple.h  simplefactory.h
SOURCES = simple.cpp simplefactory.cpp
TRANSLATABLES = $${HEADERS} $${SOURCES}

LIBS+=-lqtopiacalc

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/calculator/simple.desktop
desktop.path=/plugins/calculator/
INSTALLS += desktop

