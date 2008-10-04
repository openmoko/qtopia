requires(enable_voip)
TEMPLATE=app
TARGET=gtalksettings
CONFIG+=quicklaunch

CONFIG+=qtopia singleexec
QTOPIA*=phone

include(gtalksettings.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

