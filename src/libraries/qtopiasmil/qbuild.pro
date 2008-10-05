TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopiasmil
VERSION=4.0.0

MODULE_NAME=qtopiasmil

enable_qtopiamedia:QTOPIA*=media

include(qtopiasmil.pro)
