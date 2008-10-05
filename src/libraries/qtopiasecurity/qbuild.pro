requires(enable_sxe)
TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopiasecurity
VERSION=4.0.0

MODULE_NAME=qtopiasecurity
QTOPIA=base
CONFIG+=i18n use_default_i18n_rules

include(qtopiasecurity.pro)

