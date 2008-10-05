!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiamail
CONFIG+=qtopia_visibility
depends(libraries/qtopiapim)
equals(QTOPIA_UI,home):depends(libraries/homeui)
}

enable_cell:contains(PROJECTS,libraries/qtopiasmil):CONFIG+=enable_mms
else:DEFINES+=QTOPIA_NO_SMS QTOPIA_NO_MMS

enable_mms {
    qtopia_depot:DEFINES+=ENABLE_UNCONDITIONAL_MMS_SEND
}

RESOURCES = qtopiamail.qrc

HEADERS=\
    qmailaddress.h\
    qmailcodec.h\
    qmaildatacomparator.h\
    qmailmessage.h\
    qmailtimestamp.h\
    qmailfolder.h\
    qmailfolderkey.h\
    qmailfoldersortkey.h\
    qmailstore.h\
    qmailmessagekey.h\
    qmailmessageserver.h\
    qmailmessagesortkey.h\
    qmailcomposer.h\
    qmailcomposerplugin.h\
    qmailviewer.h\
    qmailviewerplugin.h\
    qmailid.h\
    qmailaccount.h\
    qmailmessagelistmodel.h\
    qmailaccountlistmodel.h\
    qmailmessagedelegate.h\
    qmailaccountkey.h\
    qmailaccountsortkey.h\
    qmailmessageremovalrecord.h\
    qmailserviceaction.h\
    qmailnewmessagehandler.h\
    qmailmessageset.h

PRIVATE_HEADERS=\
    bind_p.h\
    qmailfolderkey_p.h\
    qmailfoldersortkey_p.h\
    qmailstore_p.h\
    qmailmessagekey_p.h\
    qmailmessagesortkey_p.h\
    qprivateimplementation_p.h\
    qprivateimplementationdef_p.h\
    qmailaccountkey_p.h\
    qmailaccountsortkey_p.h\
    qmailkeyargument_p.h\
    semaphore_p.h

SEMI_PRIVATE_HEADERS=\
    accountconfiguration_p.h\
    longstream_p.h\
    longstring_p.h \
    detailspage_p.h\
    addressselectorwidget_p.h

SOURCES=\
    accountconfiguration.cpp\
    qmailaddress.cpp\
    qmailcodec.cpp\
    qmaildatacomparator.cpp\
    qmailmessage.cpp\
    qmailtimestamp.cpp\
    longstring.cpp \
    longstream.cpp\
    qmailfolder.cpp\
    qmailfolderkey.cpp\
    qmailfoldersortkey.cpp\
    qmailmessageserver.cpp\
    qmailmessagesortkey.cpp\
    qmailstore.cpp\
    qmailstore_p.cpp\
    qmailmessagekey.cpp\
    qmailcomposer.cpp\
    qmailcomposerplugin.cpp\
    qmailviewer.cpp\
    qmailviewerplugin.cpp\
    qmailid.cpp\
    qprivateimplementation.cpp\
    qmailaccount.cpp\
    qmailmessagelistmodel.cpp\
    qmailaccountlistmodel.cpp\
    qmailmessagedelegate.cpp\
    qmailaccountkey.cpp\
    qmailaccountsortkey.cpp\
    qmailmessageremovalrecord.cpp\
    detailspage.cpp\
    semaphore.cpp\
    addressselectorwidget.cpp\
    qmailserviceaction.cpp\
    qmailnewmessagehandler.cpp\
    qmailmessageset.cpp

pkg.desc=Allows access to the Qtopia mailbox files.

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/mail
headers.hint=sdk headers
INSTALLS+=headers

pheaders.files=$$SEMI_PRIVATE_HEADERS
pheaders.path=/include/qtopia/mail/private
pheaders.hint=sdk headers
INSTALLS+=pheaders

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
