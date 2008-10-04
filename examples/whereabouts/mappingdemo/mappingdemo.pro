!qbuild{ qtopia_project(qtopia app)
TARGET=mappingdemo
CONFIG+=qtopia_main no_quicklaunch
}
# Specify the languages that make lupdate should produce .ts files for
AVAILABLE_LANGUAGES=en_US
# Specify the langauges we want to install translations for
LANGUAGES=$$AVAILABLE_LANGUAGES

HEADERS=mappingdemo.h
SOURCES=mappingdemo.cpp \
        main.cpp

desktop.files=$$QTOPIA_DEPOT_PATH/examples/whereabouts/mappingdemo/mappingdemo.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=mappingdemo
pkg.desc=Mapping Demo
pkg.domain=trusted

# Install pictures.
pics.files=pics/*
pics.path=/pics/mappingdemo
pics.hint=pics
INSTALLS+=pics

# Install sample NMEA log
nmea.files=nmea_sample.txt
nmea.path=/etc/whereabouts
INSTALLS+=nmea
!qbuild{
# need the whereabouts library
depends(libraries/qtopiawhereabouts)

# only build if whereabouts api is enabled in Qtopia
requires(enable_qtopiawhereabouts)
}
