!qbuild{
qtopia_project(qtopia app)
TARGET=messageserver
CONFIG+=singleexec_main

depends(libraries/qtopiabase)
depends(libraries/qtopiamail)

enable_telephony {
    depends(libraries/qtopiacollective)
    depends(libraries/qtopiaphone)
} else {
    DEFINES+=QTOPIA_NO_COLLECTIVE
}

enable_cell:contains(PROJECTS,libraries/qtopiasmil):CONFIG+=enable_mms
else:DEFINES+=QTOPIA_NO_SMS QTOPIA_NO_MMS
}

HEADERS+=\
        client.h\
        emailhandler.h\
        imapclient.h\
        imapprotocol.h\
        mailmessageclient.h\
        mailtransport.h\
        messagearrivalservice.h\
        messageclassifier.h\
        messageserver.h\
        popclient.h\
        smsclient.h\
        smsdecoder.h\
        smtpclient.h\
        systemclient.h

SOURCES+=\
        client.cpp\
        emailhandler.cpp\
        imapclient.cpp\
        imapprotocol.cpp\
        mailmessageclient.cpp\
        mailtransport.cpp\
        main.cpp\
        messagearrivalservice.cpp\
        messageclassifier.cpp\
        messageserver.cpp\
        popclient.cpp\
        smsclient.cpp\
        smsdecoder.cpp\
        smtpclient.cpp\
        systemclient.cpp

MMS.TYPE=CONDITIONAL_SOURCES
MMS.CONDITION=enable_mms
MMS.SOURCES=\
        mmsclient.cpp\
        mmscomms.cpp\
        mmsmessage.cpp
MMS.HEADERS=\
        mmsclient.h\
        mmscomms.h\
        mmsmessage.h
!qbuild:CONDITIONAL_SOURCES(MMS)

MMSCOMMS.TYPE=CONDITIONAL_SOURCES
MMSCOMMS.CONDITION=enable_mms
MMSCOMMS.HEADERS=mmscomms_http.h
MMSCOMMS.SOURCES=mmscomms_http.cpp
!qbuild:CONDITIONAL_SOURCES(MMSCOMMS)
enable_mms:DEFINES+=MMSCOMMS_HTTP

COLLECTIVE.TYPE=CONDITIONAL_SOURCES
COLLECTIVE.CONDITION=enable_telephony
COLLECTIVE.HEADERS=collectiveclient.h
COLLECTIVE.SOURCES=collectiveclient.cpp
!qbuild:CONDITIONAL_SOURCES(COLLECTIVE)

pkg.desc = Message Server
pkg.domain = trusted 

daemon.files=$$QTOPIA_DEPOT_PATH/etc/daemons/messageserver.conf
daemon.path=/etc/daemons
INSTALLS+=daemon

enable_cell {
    messagearrivalservice.files=$$QTOPIA_DEPOT_PATH/services/MessageArrival/messageserver
    messagearrivalservice.path=/services/MessageArrival
    INSTALLS+=messagearrivalservice

    qdsmessagearrivalservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/MessageArrival
    qdsmessagearrivalservice.path=/etc/qds
    INSTALLS+=qdsmessagearrivalservice
}

# Services used by messageserver to invoke new message handlers
#newsystemmessagearrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewSystemMessageArrival.service
#newsystemmessagearrivalservice.path=/services
#INSTALLS+=newsystemmessagearrivalservice

qdsnewsystemmessagearrivalservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/NewSystemMessageArrival
qdsnewsystemmessagearrivalservice.path=/etc/qds
INSTALLS+=qdsnewsystemmessagearrivalservice

#newemailarrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewEmailArrival.service
#newemailarrivalservice.path=/services
#INSTALLS+=newemailarrivalservice

qdsnewemailarrivalservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/NewEmailArrival
qdsnewemailarrivalservice.path=/etc/qds
INSTALLS+=qdsnewemailarrivalservice

enable_cell {
    #newsmsarrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewSmsArrival.service
    #newsmsarrivalservice.path=/services
    #INSTALLS+=newsmsarrivalservice

    qdsnewsmsarrivalservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/NewSmsArrival
    qdsnewsmsarrivalservice.path=/etc/qds
    INSTALLS+=qdsnewsmsarrivalservice

    #newmmsarrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewMmsArrival.service
    #newmmsarrivalservice.path=/services
    #INSTALLS+=newmmsarrivalservice

    qdsnewmmsarrivalservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/NewMmsArrival
    qdsnewmmsarrivalservice.path=/etc/qds
    INSTALLS+=qdsnewmmsarrivalservice
}

enable_telephony {
    #newinstantmessagearrivalservice.files=$$QTOPIA_DEPOT_PATH/services/NewInstantMessageArrival.service
    #newinstantmessagearrivalservice.path=/services
    #INSTALLS+=newinstantmessagearrivalservice

    qdsnewinstantmessagearrivalservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/NewInstantMessageArrival
    qdsnewinstantmessagearrivalservice.path=/etc/qds
    INSTALLS+=qdsnewinstantmessagearrivalservice
}

