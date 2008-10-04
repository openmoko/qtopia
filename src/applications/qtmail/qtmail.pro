!qbuild {
qtopia_project(qtopia app)
TARGET=qtmail
CONFIG+=qtopia_main
depends(libraries/qtopiamail)
depends(libraries/qtopiapim)
equals(QTOPIA_UI,home):depends(libraries/homeui)
enable_telephony:depends(libraries/qtopiaphone)
}

enable_cell:contains(PROJECTS,libraries/qtopiasmil):CONFIG+=enable_mms
else:DEFINES+=QTOPIA_NO_SMS QTOPIA_NO_MMS

!enable_telephony:DEFINES+=QTOPIA_NO_COLLECTIVE

FORMS = editaccountbasephone.ui

HEADERS+=\
    accountsettings.h\
    editaccount.h\
    emailclient.h\
    emailservice.h\
    emailpropertysetter.h\
    icontype.h\
    maillist.h\
    messagefolder.h\
    messagelistview.h\
    messagesservice.h\
    messagestore.h\
    qtmailwindow.h\
    readmail.h\
    selectfolder.h\
    smsservice.h\
    statusdisplay.h\
    viewatt.h\
    writemail.h\
    selectcomposerwidget.h\
    folderdelegate.h\
    foldermodel.h\
    folderview.h\
    actionfoldermodel.h\
    actionfolderview.h\
    emailfoldermodel.h\
    emailfolderview.h

SOURCES+=\
    accountsettings.cpp\
    editaccount.cpp\
    emailclient.cpp\
    emailservice.cpp\
    emailpropertysetter.cpp\
    maillist.cpp\
    main.cpp\
    messagefolder.cpp\
    messagelistview.cpp\
    messagesservice.cpp\
    messagestore.cpp\
    qtmailwindow.cpp\
    readmail.cpp\
    selectfolder.cpp\
    smsservice.cpp\
    statusdisplay.cpp\
    viewatt.cpp\
    writemail.cpp\
    selectcomposerwidget.cpp\
    folderdelegate.cpp\
    foldermodel.cpp\
    folderview.cpp\
    actionfoldermodel.cpp\
    actionfolderview.cpp\
    emailfoldermodel.cpp\
    emailfolderview.cpp

MMS.TYPE=CONDITIONAL_SOURCES
MMS.CONDITION=enable_mms
MMS.FORMS=mmseditaccountbase.ui
MMS.HEADERS=mmseditaccount.h
MMS.SOURCES=mmseditaccount.cpp
!qbuild:CONDITIONAL_SOURCES(MMS)

COLLECTIVE.TYPE=CONDITIONAL_SOURCES
COLLECTIVE.CONDITION=enable_telephony
COLLECTIVE.HEADERS=instantmessageservice.h
COLLECTIVE.SOURCES=instantmessageservice.cpp
!qbuild:CONDITIONAL_SOURCES(COLLECTIVE)

NONHOME.TYPE=CONDITIONAL_SOURCES
NONHOME.CONDITION=!equals(QTOPIA_UI,home)
NONHOME.FORMS=searchviewbasephone.ui
NONHOME.HEADERS=searchview.h
NONHOME.SOURCES=searchview.cpp
!qbuild:CONDITIONAL_SOURCES(NONHOME)

pics.files=$$QTOPIA_DEPOT_PATH/pics/qtmail/*
pics.path=/pics/qtmail
pics.hint=pics
INSTALLS+=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/qtmail.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
emailservice.files=$$QTOPIA_DEPOT_PATH/services/Email/qtmail
emailservice.path=/services/Email
INSTALLS+=emailservice
qdsemailservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Email
qdsemailservice.path=/etc/qds
INSTALLS+=qdsemailservice
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=qtmail*
help.hint=help
INSTALLS+=help

smsservice.files=$$QTOPIA_DEPOT_PATH/services/SMS/qtmail
smsservice.path=/services/SMS
enable_cell:INSTALLS+=smsservice
instantmessageservice.files=$$QTOPIA_DEPOT_PATH/services/InstantMessage/qtmail
instantmessageservice.path=/services/InstantMessage
enable_telephony:INSTALLS+=instantmessageservice
messageservice.files=$$QTOPIA_DEPOT_PATH/services/Messages/qtmail
messageservice.path=/services/Messages
INSTALLS+=messageservice
qdssmsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/SMS
qdssmsservice.path=/etc/qds
enable_cell:INSTALLS+=qdssmsservice

# Service entries to handle new message arrivals
newsystemmessagearrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewSystemMessageArrival/qtmail
newsystemmessagearrivalservice.path=/services/NewSystemMessageArrival
INSTALLS+=newsystemmessagearrivalservice

newemailarrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewEmailArrival/qtmail
newemailarrivalservice.path=/services/NewEmailArrival
INSTALLS+=newemailarrivalservice

enable_cell {
    newsmsarrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewSmsArrival/qtmail
    newsmsarrivalservice.path=/services/NewSmsArrival
    INSTALLS+=newsmsarrivalservice

    newmmsarrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewMmsArrival/qtmail
    newmmsarrivalservice.path=/services/NewMmsArrival
    INSTALLS+=newmmsarrivalservice
}

enable_telephony {
    newinstantmessagearrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewInstantMessageArrival/qtmail
    newinstantmessagearrivalservice.path=/services/NewInstantMessageArrival
    INSTALLS+=newinstantmessagearrivalservice
}

pkg.name=qpe-mail
pkg.desc=Messaging application for Qtopia.
pkg.domain=trusted

