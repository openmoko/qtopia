TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtopiapim
VERSION=4.0.0

MODULE_NAME=qtopiapim
QTOPIA*=comm collective
enable_cell:QTOPIA*=phone
MODULES*=sqlite vobject
CONFIG+=i18n use_default_i18n_rules

include(qtopiapim.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

