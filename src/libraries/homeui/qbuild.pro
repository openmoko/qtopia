TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=homeui
VERSION=4.0.0

MODULE_NAME=homeui
CONFIG+=i18n use_default_i18n_rules
QTOPIA*=pim

include(homeui.pro)
