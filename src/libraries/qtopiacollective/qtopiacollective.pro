!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiacollective
CONFIG+=qtopia_visibility no_tr
#depends(libraries/qtopiapim)
}

HEADERS = \
    qcollectivepresence.h \
    qcollectivepresenceinfo.h \
    qcollectivenamespace.h \
    qcollectivemessenger.h \
    qcollectivesimplemessage.h 

SEMI_PRIVATE_HEADERS = sippresencereader_p.h \
                       sippresencewriter_p.h \
                       dummypresenceservice_p.h \
                       dummypresencecontrol_p.h

SOURCES = \
    qcollectivepresence.cpp \
    qcollectivepresenceinfo.cpp \
    qcollectivenamespace.cpp \
    qcollectivemessenger.cpp \
    qcollectivesimplemessage.cpp \
    sippresencereader.cpp \
    sippresencewriter.cpp \
    dummypresencecontrol.cpp \
    dummypresenceservice.cpp

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/collective
headers.hint=headers sdk
INSTALLS+=headers

pheaders.files=$$SEMI_PRIVATE_HEADERS
pheaders.path=/include/qtopia/collective/private
pheaders.hint=sdk headers
INSTALLS+=pheaders

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
