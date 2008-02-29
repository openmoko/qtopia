qtopia_project(qtopia app)
TARGET=security
CONFIG+=qtopia_main no_quicklaunch

HEADERS		= security.h
SOURCES		= security.cpp main.cpp

enable_cell {
    FORMS	= securityphone.ui
    HEADERS	+= phonesecurity.h
    SOURCES	+= phonesecurity.cpp
    depends(libraries/qtopiaphone)
} else {
    FORMS	= securitybase.ui
}

TRANSLATABLES +=  phonesecurity.h \
                    phonesecurity.cpp \
                    securityphone.ui \
                    securitybase.ui

pics.files=$$QTOPIA_DEPOT_PATH/pics/security/*
pics.path=/pics/security
pics.hint=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/Security.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=security*
help.hint=help
INSTALLS+=pics desktop help

pkg.description=Security settings application
pkg.domain=window,phonecomm,security
