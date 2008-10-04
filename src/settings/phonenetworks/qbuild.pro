TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=phonenetworks

QTOPIA*=pim phone collective
CONFIG+=quicklaunch

include(phonenetworks.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

