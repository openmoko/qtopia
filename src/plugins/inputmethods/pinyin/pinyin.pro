!qbuild{
TARGET=pinyinim
qtopia_project(qtopia plugin)
depends(3rdparty/libraries/inputmatch)
plugin_type  = inputmethods
CONFIG+=no_tr
}

HEADERS      = pinyinim.h pinyinimpl.h
SOURCES      = pinyinim.cpp pinyinimpl.cpp

im.files=$$QTOPIA_DEPOT_PATH/etc/im/pyim/*
im.path=/etc/im/pyim
INSTALLS+=im
