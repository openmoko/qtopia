TEMPLATE=lib
CONFIG+=qt embedded hide_symbols singleexec
TARGET=qtopiagfx
VERSION=4.0.0

MODULE_NAME=qtopiagfx
DEPENDS*=/src/3rdparty/libraries/easing::persisted

include(qtopiagfx.pro)

