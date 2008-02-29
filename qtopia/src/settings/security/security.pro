CONFIG		+= qtopiaapp

HEADERS		= security.h
SOURCES		= security.cpp main.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES} 

QTOPIA_PHONE {
    HEADERS	+= phonesecurity.h
    SOURCES	+= phonesecurity.cpp
    INTERFACES	= securityphone.ui
}
else {
    INTERFACES	= securitybase.ui
}

TRANSLATABLES +=  phonesecurity.h \
                    phonesecurity.cpp \
                    securityphone.ui \
                    securitybase.ui

TARGET		= security

pics.files=$${QTOPIA_DEPOT_PATH}/pics/security/*
pics.path=/pics/security
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Settings/Security.desktop
desktop.path=/apps/Settings
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=security*
INSTALLS+= desktop
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

