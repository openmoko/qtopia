qtopia_project(qtopia app)
TARGET=dbmigrate
CONFIG+=no_tr singleexec_main

SOURCES	= \
    main.cpp \
    migrateengine.cpp\
    qtopiapim/pimmigrate.cpp

HEADERS  = \
    migrateengine.h\
    qtopiapim/pimmigrate.h

RESOURCES+=\
    qtopiapim/pimmigrate.qrc

!enable_singleexec {
    RESOURCES+=\
        $$QTOPIA_DEPOT_PATH/src/libraries/qtopia/qtopia.qrc\
        $$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim/qtopiapim.qrc
}

depends(libraries/qtopia)
depends(libraries/qtopiapim)
depends(3rdparty/libraries/sqlite)

pkg.desc=Database upgrade migration utility
pkg.domain=docapi,cardreader,prefix

dbmigrateservice.files=$$QTOPIA_DEPOT_PATH/services/DBMigrationEngine/dbmigrate
dbmigrateservice.path=/services/DBMigrationEngine
INSTALLS+=dbmigrateservice

dbmigrateDSservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/DBMigrationEngine
dbmigrateDSservice.path=/etc/qds
INSTALLS+=dbmigrateDSservice
