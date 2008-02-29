qtopia_project(qtopia app)
TARGET=qtmail
CONFIG+=qtopia_main

phone:contains(PROJECTS,libraries/qtopiasmil):CONFIG+=enable_mms
PHONE_FORMS_MMS = mmseditaccountbase.ui
PHONE_FORMS = editaccountbasephone.ui searchviewbasephone.ui

phoneoff=0
!enable_modem:phoneoff=1
free_package:!free_plus_binaries:phoneoff=1
equals(phoneoff,1) {
    CONFIG-=enable_mms
    DEFINES+=QTOPIA_NO_SMS
}

enable_mms:PHONE_FORMS += $$PHONE_FORMS_MMS
PDA_FORMS   = editaccountbase.ui searchviewbase.ui writemailbase.ui

phone {
    FORMS += $$PHONE_FORMS
} else {
    FORMS += $$PDA_FORMS
}

HEADERS+=\
    emailclient.h\
    emailhandler.h\
    emaillistitem.h\
    emailfolderlist.h\
    popclient.h\
    readmail.h\
    smtpclient.h\
    writemail.h\
    viewatt.h\
    addatt.h\
    editaccount.h\
    maillist.h\
    searchview.h\
    search.h\
    maillistview.h\
    email.h\
    account.h\
    imapclient.h\
    folder.h\
    folderlistview.h\
    accountlist.h\
    imapprotocol.h\
    qtmailwindow.h\
    common.h\
    client.h\
    qtmailgui.h

SOURCES+=\
    emailclient.cpp\
    emailhandler.cpp\
    emaillistitem.cpp\
    emailfolderlist.cpp\
    popclient.cpp\
    readmail.cpp\
    smtpclient.cpp\
    writemail.cpp\
    viewatt.cpp\
    addatt.cpp\
    editaccount.cpp\
    maillist.cpp\
    searchview.cpp\
    search.cpp\
    maillistview.cpp\
    email.cpp\
    account.cpp\
    imapclient.cpp\
    folder.cpp\
    folderlistview.cpp\
    accountlist.cpp\
    imapprotocol.cpp\
    qtmailwindow.cpp\
    common.cpp\
    client.cpp\
    qtmailgui.cpp\
    main.cpp

PHONE_SOURCES=\
    smsclient.cpp\
    audiosource.cpp\
    smsdecoder.cpp\
    accountsettings.cpp\
    genericcomposer.cpp\
    detailspage.cpp\
    composer.cpp\
    emailcomposer.cpp\
    templatetext.cpp\
    selectfolder.cpp

MMS_SOURCES=\
    mmsclient.cpp\
    mmscomms.cpp\
    mmscomposer.cpp\
    mmseditaccount.cpp\
    mmsmessage.cpp
                    
PHONE_HEADERS=\
    smsclient.h\
    audiosource.h\
    smsdecoder.h\
    accountsettings.h\
    addattdialogphone.h\
    genericcomposer.h\
    detailspage.h\
    composer.h\
    emailcomposer.h\
    templatetext.h\
    selectfolder.h

MMS_HEADERS=\
    mmsclient.h\
    mmscomms.h\
    mmscomposer.h\
    mmseditaccount.h\
    mmsmessage.h

enable_mms:PHONE_HEADERS+=$$MMS_HEADERS
enable_mms:PHONE_SOURCES+=$$MMS_SOURCES

!enable_singleexec {
    PHONE_HEADERS+=\
        ../addressbook/imagesourcedialog.h \
        ../mediarecorder/audioparameters.h
    PHONE_SOURCES+=\
        ../addressbook/imagesourcedialog.cpp \
        ../mediarecorder/audioparameters.cpp
}
enable_singleexec:!contains(PROJECTS,applications/addressbook) {
    PHONE_HEADERS+=\
        ../addressbook/imagesourcedialog.h \
        ../mediarecorder/audioparameters.h
    PHONE_SOURCES+=\
        ../addressbook/imagesourcedialog.cpp \
        ../mediarecorder/audioparameters.cpp
}

PDA_SOURCES=\
    pdacomposer.cpp
PDA_HEADERS=\
    addattdialogpda.h\
    pdacomposer.h

phone {
    SOURCES+=$$PHONE_SOURCES
    HEADERS+=$$PHONE_HEADERS
} else {
    SOURCES+=$$PDA_SOURCES
    HEADERS+=$$PDA_HEADERS
}

enable_mms {
# To enable HTTP MMS comms:
    MMSCOMMS_HEADERS=mmscomms_http.cpp
    MMSCOMMS_SOURCES=mmscomms_http.h
    SOURCES+=$$MMSCOMMS_HEADERS
    HEADERS+=$$MMSCOMMS_SOURCES

    DEFINES+=MMSCOMMS_HTTP
}

TRANSLATABLES +=    $$PHONE_HEADERS\
                    $$PHONE_SOURCES\
                    $$PDA_HEADERS\
                    $$PDA_SOURCES\
                    $$MMS_HEADERS\
                    $$MMS_SOURCES\
                    $$PHONE_FORMS\
                    $$PDA_FORMS\
                    $$PHONE_FORMS_MMS\
                    $$MMSCOMMS_HEADERS\
                    $$MMSCOMMS_SOURCES

enable_ssl {
    DEFINES+=SMTPAUTH
}

depends(libraries/qtopiamail)
depends(libraries/qtopiapim)
enable_mms:depends(libraries/qtopiasmil)
else:DEFINES+=QTOPIA_NO_MMS

pics.files=$$QTOPIA_DEPOT_PATH/pics/qtmail/*
pics.path=/pics/qtmail
pics.hint=pics
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/qtmail.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
emailservice.files=$$QTOPIA_DEPOT_PATH/services/Email/qtmail
emailservice.path=/services/Email
qdsemailservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Email
qdsemailservice.path=/etc/qds
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=qtmail*
help.hint=help
INSTALLS+=pics desktop emailservice qdsemailservice help

phone {
    smsservice.files=$$QTOPIA_DEPOT_PATH/services/SMS/qtmail
    smsservice.path=/services/SMS
    messageservice.files=$$QTOPIA_DEPOT_PATH/services/Messages/qtmail
    messageservice.path=/services/Messages
    qdssmsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/SMS
    qdssmsservice.path=/etc/qds

    INSTALLS+=messageservice smsservice qdssmsservice
}

pkg.name=qpe-mail
pkg.desc=Messaging application for Qtopia.
pkg.domain=window,docapi,qds,msg,phonecomm,cardreader,camera,pictures,alarm,mediarecorder,pim,screensaver,net,drm,print
