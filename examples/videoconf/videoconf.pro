!qbuild {
qtopia_project(qtopia app)
TARGET=videoconf
depends(libraries/qtopiamedia)
depends(libraries/qtopiavideo)
}

# Input
HEADERS += conferencewidget.h payloadmodel.h callcontrols.h
SOURCES += conferencewidget.cpp \
           main.cpp \
           payloadmodel.cpp \
           callcontrols.cpp

desktop.files=videoconf.desktop
desktop.path=/apps/Applications
desktop.trtarget=videoconf-nct
desktop.hint=nct desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics/videoconf
pics.hint=pics
INSTALLS+=pics

# Set this to trusted for full privileges
target.hint=sxe
target.domain=trusted

