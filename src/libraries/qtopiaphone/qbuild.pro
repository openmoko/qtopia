TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopiaphone
VERSION=4.0.0

MODULE_NAME=qtopiaphone
QTOPIA*=comm
CONFIG+=i18n use_default_i18n_rules

include(qtopiaphone.pro)

