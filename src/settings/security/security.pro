!qbuild {
qtopia_project(qtopia app)
TARGET=security
CONFIG+=qtopia_main
enable_cell:depends(libraries/qtopiaphone)
}

HEADERS		= security.h
SOURCES		= security.cpp main.cpp

CELL.TYPE=CONDITIONAL_SOURCES
CELL.CONDITION=enable_cell
CELL.FORMS=securityphone.ui
CELL.HEADERS=phonesecurity.h
CELL.SOURCES=phonesecurity.cpp
!qbuild:CONDITIONAL_SOURCES(CELL)

NONCELL.TYPE=CONDITIONAL_SOURCES
NONCELL.CONDITION=!enable_cell
NONCELL.FORMS=securitybase.ui
!qbuild:CONDITIONAL_SOURCES(NONCELL)

pics.files=$$QTOPIA_DEPOT_PATH/pics/security/*
pics.path=/pics/security
pics.hint=pics
INSTALLS+=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Settings/security.desktop
desktop.path=/apps/Settings
desktop.hint=desktop
INSTALLS+=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=security*
help.hint=help
INSTALLS+=help

pkg.description=Security settings application
pkg.domain=trusted
