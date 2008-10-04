TEMPLATE=app
CONFIG+=qtopia quicklaunch singleexec
TARGET=startupflags

HEADERS		= startupflags.h
SOURCES		= startupflags.cpp main.cpp

pkg.desc=Startup flags editor for Qtopia.
pkg.domain=trusted

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/startupflags.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop

conf.files=$$device_overrides(/etc/default/Trolltech/StartupFlags.conf)
conf.path=/etc/default/Trolltech
INSTALLS+=conf

script.files=$$PWD/startupflags.sh
script.path=/bin
script.hint=script
INSTALLS+=script

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

