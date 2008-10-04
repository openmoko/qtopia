!qbuild{
qtopia_project(qtopia app)
TARGET=pimdata
CONFIG+=qtopia_main no_tr no_quicklaunch no_singleexec
depends(libraries/qtopiapim)
}

DEFINES += PIMXML_NAMESPACE=PimDataGen

# Input
SOURCES += main.cpp cgen.cpp generator.cpp ../qdsync/qpimxml.cpp
HEADERS += cgen.h generator.h ../qdsync/qpimxml_p.h

RESOURCES += namefiles.qrc


pkg.desc=An application designed to aid testing of pim applications by generating large initial data sets.
pkg.domain=trusted
