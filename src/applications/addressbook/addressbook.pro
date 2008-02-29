qtopia_project(qtopia app)
TARGET=addressbook
CONFIG+=qtopia_main

HEADERS+=\
    abeditor.h\
    imagesourcedialog.h\
    ablabel.h\
    contactsource.h\
    addressbook.h

SOURCES+=\
    abeditor.cpp\
    imagesourcedialog.cpp\
    ablabel.cpp\
    addressbook.cpp\
    contactsource.cpp\
    main.cpp

phone {
    !free_package|free_plus_binaries:depends(libraries/qtopiaphone)

    SOURCES += emaildialogphone.cpp
    HEADERS += emaildialogphone.h
} else {
    FORMS += emaildlg.ui
    HEADERS += emaildlgimpl.h
    SOURCES += emaildlgimpl.cpp
}

enable_cell {
    !enable_singleexec {
        SOURCES += ../../settings/ringprofile/ringtoneeditor.cpp
        HEADERS += ../../settings/ringprofile/ringtoneeditor.h
    }
}

TRANSLATABLES += emaildialogphone.cpp \
                    emaildialogphone.h \
                    emaildlg.ui \
                    emaildlgimpl.h \
                    emaildlgimpl.cpp \
                    ../../settings/ringprofile/ringtoneeditor.cpp \
                    ../../settings/ringprofile/ringtoneeditor.h 

depends(libraries/qtopiapim)

service.files=$$QTOPIA_DEPOT_PATH/services/Contacts/addressbook
service.path=/services/Contacts
receiveservice.files=$$QTOPIA_DEPOT_PATH/services/Receive/text/x-vcard/addressbook
receiveservice.path=/services/Receive/text/x-vcard/
desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/addressbook.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=addressbook*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/addressbook/*
pics.path=/pics/addressbook
pics.hint=pics
im.files=named_addressbook-*.conf
im.path=/etc/im/pkim
phoneservice.files=$$QTOPIA_DEPOT_PATH/services/ContactsPhone/addressbook
phoneservice.path=/services/ContactsPhone
qdlservice.files=$$QTOPIA_DEPOT_PATH/services/QDL/addressbook
qdlservice.path=/services/QDL
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Contacts
qdsservice.path=/etc/qds
qdsphoneservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/ContactsPhone
qdsphoneservice.path=/etc/qds
INSTALLS+=service receiveservice desktop help pics im qdlservice qdsservice
phone {
    INSTALLS+=phoneservice qdsphoneservice
}

pkg.desc=Contacts for Qtopia.
pkg.domain=pim,window,qdl,qds,beaming,phonecomm,pictures,msg,docapi,cardreader,camera,pictures,mediarecorder
