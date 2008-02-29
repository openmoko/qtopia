qtopia_project(qtopia app)
TARGET=sysinfo
CONFIG+=qtopia_main

#DEFINES         += SYSINFO_GEEK_MODE

HEADERS		= memory.h \
		  graph.h \
		  load.h \
		  storage.h \
		  versioninfo.h \
		  sysinfo.h \
                  dataview.h \
                  securityinfo.h

#phone{
#    HEADERS     += cleanupwizard.h
#} else {
#    HEADERS     += cleanupwizard_pda.h
#}

SOURCES		= memory.cpp \
		  graph.cpp \
		  load.cpp \
		  storage.cpp \
		  versioninfo.cpp \
		  sysinfo.cpp \
		  main.cpp \
                  dataview.cpp \
                  securityinfo.cpp
#                  cleanupwizard.cpp

enable_cell {
    HEADERS += siminfo.h modeminfo.h
    SOURCES += siminfo.cpp modeminfo.cpp
    depends(libraries/qtopiaphone)
}

TRANSLATABLES   += \
                    cleanupwizard.h \
                    cleanupwizard_pda.h

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=sysinfo*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/sysinfo/*
pics.path=/pics/sysinfo
pics.hint=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/sysinfo.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
cleanupservice.files=$$QTOPIA_DEPOT_PATH/services/CleanupWizard/sysinfo
cleanupservice.path=/services/CleanupWizard
INSTALLS+=help pics desktop cleanupservice

pkg.desc=System information for Qtopia.
pkg.domain=window,docapi,cardreader,phonecomm
