!qbuild {
qtopia_project(qtopia app)
TARGET=sysinfo
CONFIG+=qtopia_main
# Always rebuild versioninfo.o so that the reported build date is correct
create_raw_dependency($$OBJECTS_DIR/versioninfo.o,FORCE)
depends(libraries/qtopiacomm)
enable_cell:depends(libraries/qtopiaphone)
}

FORMS = documenttypeselector.ui

HEADERS		= memory.h \
		  graph.h \
		  load.h \
		  storage.h \
		  versioninfo.h \
		  sysinfo.h \
          dataview.h \
          securityinfo.h \
          networkinfo.h \
          cleanupwizard.h \
          deviceselector.h \

SOURCES		= memory.cpp \
		  graph.cpp \
		  load.cpp \
		  storage.cpp \
		  versioninfo.cpp \
		  sysinfo.cpp \
		  main.cpp \
          dataview.cpp \
          securityinfo.cpp \
          networkinfo.cpp \
          cleanupwizard.cpp \

CELL.TYPE=CONDITIONAL_SOURCES
CELL.CONDITION=enable_cell
CELL.HEADERS=siminfo.h modeminfo.h
CELL.SOURCES=siminfo.cpp modeminfo.cpp
!qbuild:CONDITIONAL_SOURCES(CELL)

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=sysinfo*
help.hint=help
INSTALLS+=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/sysinfo/*
pics.path=/pics/sysinfo
pics.hint=pics
INSTALLS+=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/sysinfo.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
cleanupservice.files=$$QTOPIA_DEPOT_PATH/services/CleanupWizard/sysinfo
cleanupservice.path=/services/CleanupWizard
INSTALLS+=cleanupservice

pkg.desc=System information for Qtopia.
pkg.domain=trusted
