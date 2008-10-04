TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=packagemanager

MODULES*=tar md5
enable_sxe:QTOPIA*=security
CONFIG+=quicklaunch

include(packagemanager.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

