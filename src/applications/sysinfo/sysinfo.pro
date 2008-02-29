CONFIG		+= qtopiaapp

HEADERS		= memory.h \
		  graph.h \
		  load.h \
		  storage.h \
		  versioninfo.h \
		  sysinfo.h \
                  dataview.h 

QTOPIA_PHONE{
    HEADERS     += cleanupwizard.h
} else {
    HEADERS     += cleanupwizard_pda.h
}

SOURCES		= memory.cpp \
		  graph.cpp \
		  load.cpp \
		  storage.cpp \
		  versioninfo.cpp \
		  sysinfo.cpp \
		  main.cpp \
                  dataview.cpp \
                  cleanupwizard.cpp



                  
TARGET		= sysinfo

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \
                    cleanupwizard.h \
                    cleanupwizard_pda.h

help.files=$${QTOPIA_DEPOT_PATH}/help/html/sysinfo*
help.path=/help/html

pics.files=$${QTOPIA_DEPOT_PATH}/pics/sysinfo/*
pics.path=/pics/sysinfo
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/sysinfo.desktop
desktop.path=/apps/Applications
cleanupservice.files=$${QTOPIA_DEPOT_PATH}/services/CleanupWizard/sysinfo
cleanupservice.path=/services/CleanupWizard

INSTALLS+=desktop help cleanupservice
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=System information dialog for the Qtopia environment.
