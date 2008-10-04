TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=phonesettings

QTOPIA*=pim phone
CONFIG+=quicklaunch

include(phonesettings.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

