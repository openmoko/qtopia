TEMPLATE=app
CONFIG+=qtopia singleexec
TARGET=messageserver

QTOPIA*=mail

enable_cell:contains(PROJECTS,libraries/qtopiasmil):CONFIG+=enable_mms
else:DEFINES+=QTOPIA_NO_SMS QTOPIA_NO_MMS

enable_telephony {
    QTOPIA*=collective phone
} else {
    DEFINES+=QTOPIA_NO_COLLECTIVE
}

include(messageserver.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

