CONFIG		+= qtopiaapp 

VPATH		+= $$QTOPIA_DEPOT_PATH/src/applications/addressbook/


HEADERS		+= abeditor.h\
		    imagesourcedialog.h

SOURCES		+= abeditor.cpp\
		    imagesourcedialog.cpp

TRANSLATABLES += $${HEADERS} $${SOURCES} 

!qtopiadesktop:HEADERS += imagesource.h \
			sendcontactservice.h \
			ablabel.h
!qtopiadesktop:SOURCES += imagesource.cpp \
			sendcontactservice.cpp \
			ablabel.cpp

TRANSLATABLES += imagesource.h \
			sendcontactservice.h \
			ablabel.h \
                 imagesource.cpp \
			sendcontactservice.cpp \
			ablabel.cpp
                        
QTOPIA_PHONE:!qtopiadesktop {
    SOURCES += emaildialogphone.cpp
    HEADERS += emaildialogphone.h
}
else {
    INTERFACES += emaildlg.ui
    HEADERS += emaildlgimpl.h
    SOURCES += emaildlgimpl.cpp
}

QTOPIA_PHONE:!qtopiadesktop:!buildSingleexec {
    SOURCES += ../../settings/ringprofile/ringtoneeditor.cpp
    HEADERS += ../../settings/ringprofile/ringtoneeditor.h
    SOURCES += ../../server/phone/phoneprofile.cpp
    HEADERS += ../../server/phone/phoneprofile.h
}

TRANSLATABLES += emaildialogphone.cpp \
                    emaildialogphone.h \
                    emaildlg.ui \
                    emaildlgimpl.h \
                    emaildlgimpl.cpp \
                    ../../settings/ringprofile/ringtoneeditor.cpp \
                    ../../settings/ringprofile/ringtoneeditor.h \
                    ../../server/phone/phoneprofile.cpp \
                    ../../server/phone/phoneprofile.h 

TARGET		= addressbook


