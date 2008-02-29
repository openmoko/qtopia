qtopia_project(qtopia app)
TARGET=mediarecorder
CONFIG+=qtopia_main

FORMS      = mediarecorderbase.ui confrecorderbase.ui
HEADERS		= mediarecorder.h pluginlist.h \
		  samplebuffer.h timeprogressbar.h confrecorder.h waveform.h \
                  audioparameters.h
SOURCES		= mediarecorder.cpp pluginlist.cpp samplebuffer.cpp \
                  timeprogressbar.cpp confrecorder.cpp waveform.cpp main.cpp \
                  audioparameters.cpp

depends(libraries/qtopiaaudio)

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/mediarecorder.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=mediarecorder*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/mediarecorder/*
pics.path=/pics/mediarecorder
pics.hint=pics
voicerecorderservice.files=$$QTOPIA_DEPOT_PATH/services/VoiceRecording/mediarecorder
voicerecorderservice.path=/services/VoiceRecording
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/VoiceRecording
qdsservice.path=/etc/qds
INSTALLS+=desktop help pics voicerecorderservice qdsservice

# the server does this for us
#categories.files=$$QTOPIA_DEPOT_PATH/etc/categories/mediarecorder.conf
#categories.trtarget=QtopiaCategories
#categories.hint=nct
#INSTALLS+=categories

pkg.desc=Media recorder for Qtopia.
pkg.domain=window,qds,mediarecorder,mediasession,screensaver,cardreader,docapi{video/*:image/*:audio/*}
