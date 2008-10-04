TEMPLATE=app
CONFIG+=qtopia
TARGET=videoconf

CONFIG+=quicklaunch singleexec

MODULES *= qtopiamedia \
           qtopiavideo

include(videoconf.pro)

#No translations for this example
#STRING_LANGUAGE=en_US
#AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
#LANGUAGES=$$QTOPIA_LANGUAGES
