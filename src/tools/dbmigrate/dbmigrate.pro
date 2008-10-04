!qbuild{
qtopia_project(qtopia plugin)
TARGET=dbmigrate
plugin_type = qtopiasqlmigrate
depends(libraries/qtopia)
depends(libraries/qtopiabase)
depends(3rdparty/libraries/sqlite)
}

SOURCES	= \
    migrateengine.cpp\
    qtopiapim/pimmigrate.cpp\
    qtopiaphone/phonemigrate.cpp

HEADERS  = \
    migrateengine.h\
    qtopiapim/pimmigrate.h\
    qtopiaphone/phonemigrate.h

RESOURCES+=\
    qtopiapim/pimmigrate.qrc\
    qtopiaphone/phonemigrate.qrc

qtopia.TYPE=CONDITIONAL_SOURCES
qtopia.CONDITION=!enable_singleexec
qtopia.RESOURCES=\
    $$QTOPIA_DEPOT_PATH/src/libraries/qtopia/qtopia.qrc\
    $$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim/qtopiapim.qrc
!qbuild:CONDITIONAL_SOURCES(qtopia)

pkg.desc=Database upgrade migration utility
pkg.domain=trusted

dbmigrateservice.files=$$QTOPIA_DEPOT_PATH/services/DBMigrationEngine/dbmigrate
dbmigrateservice.path=/services/DBMigrationEngine
INSTALLS+=dbmigrateservice

dbmigrateDSservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/DBMigrationEngine
dbmigrateDSservice.path=/etc/qds
INSTALLS+=dbmigrateDSservice
