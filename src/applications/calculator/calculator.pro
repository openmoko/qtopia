!qbuild {
qtopia_project(qtopia app)
TARGET=calculator
CONFIG+=qtopia_main
}

# library files
HEADERS = calculator.h engine.h data.h instruction.h display.h \
            interfaces/stdinputwidgets.h integerdata.h integerinstruction.h
SOURCES = calculator.cpp engine.cpp data.cpp instruction.cpp \
            display.cpp interfaces/stdinputwidgets.cpp main.cpp

# double type
HEADERS += doubledata.h
SOURCES += doubledata.cpp

# Phone UI
HEADERS += interfaces/phone.h phoneinstruction.h \
                doubleinstruction.h interfaces/simple.h
SOURCES += interfaces/phone.cpp phoneinstruction.cpp \
                doubleinstruction.cpp interfaces/simple.cpp
FORMS+=helperpanel.ui

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/calculator.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=calculator*
help.hint=help
INSTALLS+=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/calculator/*
pics.path=/pics/calculator
pics.hint=pics
INSTALLS+=pics

pkg.name=qpe-calculator
pkg.desc=A simple calculator for Qtopia.
pkg.multi=libraries/qtopiacalc
pkg.domain=trusted

