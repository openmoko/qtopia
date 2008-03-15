qtopia_project(qtopia app)
TARGET=filter
CONFIG+=qtopia_main no_quicklaunch

# Specify the languages that make lupdate should produce .ts files for
AVAILABLE_LANGUAGES=en_US
# Specify the langauges we want to install translations for
LANGUAGES=$$AVAILABLE_LANGUAGES

HEADERS=filter.h
SOURCES=filter.cpp main.cpp

desktop.files=filter.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=filter
pkg.desc=Bluetooth device filter example
pkg.domain=trusted

# depends on the qtopia bluetooth libraries
depends(libraries/qtopiacomm)

# only build if bluetooth is enabled in Qtopia
requires(enable_bluetooth)
