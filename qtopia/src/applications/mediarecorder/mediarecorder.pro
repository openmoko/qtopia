CONFIG		+= qtopiaapp
HEADERS		= audioinput.h mediarecorder.h pluginlist.h \
		  samplebuffer.h timeprogressbar.h confrecorder.h waveform.h
SOURCES		= audioinput.cpp mediarecorder.cpp \
		  pluginlist.cpp samplebuffer.cpp timeprogressbar.cpp \
		  confrecorder.cpp waveform.cpp main.cpp

INTERFACES      = mediarecorderbase.ui confrecorderbase.ui

TARGET		= mediarecorder

DEPENDS         += mediaplayerbase
INCLUDEPATH     += ../../libraries/mediaplayer
DEPENDPATH      += ../../libraries/mediaplayer

LIBS            += -lmediaplayer -lpthread

TRANSLATABLES   = $$HEADERS \
                    $$SOURCES \
                    $$INTERFACES

i18n.path=$${INSTALL_PREFIX}/i18n
i18n.commands=$${COMMAND_HEADER}\
    TRANSLATIONS="$$TRANSLATIONS"; for lang in \$$TRANSLATIONS; \
    do \
	for pkg in Categories-mediarecorder; \
	do \
	    $${DQTDIR}/bin/lrelease $${QTOPIA_DEPOT_PATH}/i18n/\$$lang/\$$pkg.ts \
		-qm $(INSTALL_ROOT)/i18n/\$$lang/\$$pkg.qm; \
	done; \
    done
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/voicerecorder.desktop
desktop.path=/apps/Applications
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=mediarecorder*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/mediarecorder/*
pics.path=/pics/mediarecorder
service.files=$${QTOPIA_DEPOT_PATH}/services/GetValue/audio/mediarecorder
service.path=/services/GetValue/audio
INSTALLS+=desktop service
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
!isEmpty(DQTDIR):INSTALLS+=i18n

PACKAGE_DESCRIPTION=The multimedia recorder for the Qtopia environment.
