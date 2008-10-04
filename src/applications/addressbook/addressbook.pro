!qbuild {
qtopia_project(qtopia app)
TARGET=addressbook
CONFIG+=qtopia_main
enable_telephony:depends(libraries/qtopiaphone)
depends(libraries/qtopiapim)
depends(libraries/qtopiamail)
depends(libraries/qtopiacollective)
equals(QTOPIA_UI,home):depends(libraries/homeui)
}

HEADERS+=\
    abeditor.h\
    contactdetails.h\
    contactdocument.h\
    contactbrowser.h\
    contactmessagehistorylist.h\
    contactoverview.h\
    contactlistpane.h\
    addressbook.h\
    groupview.h\
    fieldlist.h\
    emaildialogphone.h

SOURCES+=\
    abeditor.cpp\
    contactdetails.cpp\
    addressbook.cpp\
    groupview.cpp\
    contactdocument.cpp\
    contactmessagehistorylist.cpp\
    contactbrowser.cpp\
    contactoverview.cpp\
    contactlistpane.cpp\
    emaildialogphone.cpp\
    fieldlist.cpp\
    main.cpp\

FORMS += actiondialog.ui


TEL.TYPE=CONDITIONAL_SOURCES
TEL.CONDITION=enable_telephony
TEL.HEADERS=contactcallhistorylist.h
TEL.SOURCES=contactcallhistorylist.cpp
!qbuild:CONDITIONAL_SOURCES(TEL)

HOME.TYPE=CONDITIONAL_SOURCES
HOME.CONDITION=equals(QTOPIA_UI,home)
HOME.HEADERS=deskphonedetails.h deskphonewidgets.h deskphoneeditor.h
HOME.SOURCES=deskphonedetails.cpp deskphonewidgets.cpp deskphoneeditor.cpp
!qbuild:CONDITIONAL_SOURCES(HOME)

dynamic.TYPE=CONDITIONAL_SOURCES
dynamic.CONDITION=!enable_singleexec
dynamic.HEADERS=\
    ../todolist/reminderpicker.h\
    ../todolist/qdelayedscrollarea.h
dynamic.SOURCES=\
    ../todolist/reminderpicker.cpp\
    ../todolist/qdelayedscrollarea.cpp
!qbuild:CONDITIONAL_SOURCES(dynamic)

dynamic_cell.TYPE=CONDITIONAL_SOURCES
dynamic_cell.CONDITION=!enable_singleexec:enable_telephony
dynamic_cell.HEADERS=\
    ../../settings/profileedit/ringtoneeditor.h
dynamic_cell.SOURCES=\
    ../../settings/profileedit/ringtoneeditor.cpp
!qbuild:CONDITIONAL_SOURCES(dynamic_cell)

service.files=$$QTOPIA_DEPOT_PATH/services/Contacts/addressbook
service.path=/services/Contacts
INSTALLS+=service

receiveservice.files=$$QTOPIA_DEPOT_PATH/services/Receive/text/x-vcard/addressbook
receiveservice.path=/services/Receive/text/x-vcard/
INSTALLS+=receiveservice

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/addressbook.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=addressbook*
help.hint=help
INSTALLS+=help

# pics are installed by libqtopiapim since they're shared

im.files=named_addressbook-*.conf
im.path=/etc/im/pkim
INSTALLS+=im

qdlservice.files=$$QTOPIA_DEPOT_PATH/services/QDL/addressbook
qdlservice.path=/services/QDL
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Contacts
qdsservice.path=/etc/qds
INSTALLS+=qdsservice

enable_cell {
    phoneservice.files=$$QTOPIA_DEPOT_PATH/services/ContactsPhone/addressbook
    phoneservice.path=/services/ContactsPhone
    INSTALLS+=phoneservice

    qdsphoneservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/ContactsPhone
    qdsphoneservice.path=/etc/qds
    INSTALLS+=qdsphoneservice
}

ribbonconf.files=$$device_overrides(/etc/default/Trolltech/AlphabetRibbonLayout.conf)

!isEmpty(ribbonconf.files) {
    ribbonconf.path=/etc/default/Trolltech
    INSTALLS+=ribbonconf
}

pkg.desc=Contacts for Qtopia.
pkg.domain=trusted
