!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiapim
CONFIG+=qtopia_visibility

VERSION         = 4.0.0

enable_cell:depends(libraries/qtopiaphone)
depends(libraries/qtopiacomm)
depends(libraries/qtopiacollective)
depends(3rdparty/libraries/sqlite)
depends(3rdparty/libraries/vobject)
}

RESOURCES = qtopiapim.qrc

HEADERS+=\
    qpimrecord.h\
    qtask.h\
    qappointment.h\
    qcontact.h\
    qpimmodel.h\
    qtaskmodel.h\
    qappointmentmodel.h\
    qcontactmodel.h\
    qappointmentview.h\
    qcontactview.h\
    qtaskview.h\
    qpimsource.h\
    qpimsourcemodel.h\
    qpimsourcedialog.h\
    qpimdelegate.h\
    qphonenumber.h\
    qfielddefinition.h

PRIVATE_HEADERS+=\
    qannotator_p.h\
    qsqlpimtablemodel_p.h\
    qappointmentsqlio_p.h\
    qpreparedquery_p.h\
    qtasksqlio_p.h\
    qcontactsqlio_p.h\
    qdependentcontexts_p.h\
    qpimdependencylist_p.h

SOURCES+=\
    qannotator.cpp\
    qsqlpimtablemodel.cpp\
    qpreparedquery.cpp\
    qpimrecord.cpp\
    qtask.cpp\
    qappointment.cpp\
    qappointmentsqlio.cpp\
    qcontact.cpp\
    qpimmodel.cpp\
    qtaskmodel.cpp\
    qcontactmodel.cpp\
    qappointmentmodel.cpp\
    qappointmentview.cpp\
    qcontactview.cpp\
    qtaskview.cpp\
    qpimsource.cpp\
    qpimsqlio.cpp\
    qtasksqlio.cpp\
    qcontactsqlio.cpp\
    qpimsourcemodel.cpp\
    qpimsourcedialog.cpp\
    qphonenumber.cpp\
    qpimdelegate.cpp\
    qdependentcontexts.cpp\
    qfielddefinition.cpp\
    qpimdependencylist.cpp


SEMI_PRIVATE_HEADERS+=qgooglecontext_p.h qpimsqlio_p.h
SOURCES+=qgooglecontext.cpp
#DEFINES+=GOOGLE_CALENDAR_CONTEXT

CELL.TYPE=CONDITIONAL_SOURCES
CELL.CONDITION=enable_cell
CELL.SEMI_PRIVATE_HEADERS=qsimcontext_p.h qsimsync_p.h
CELL.SOURCES=qsimcontext.cpp qsimsync.cpp
!qbuild:CONDITIONAL_SOURCES(CELL)

pkg.desc=PIM Data access library
pkg.domain=trusted

pkg_qtopiapim_settings.files=$$device_overrides(/etc/default/Trolltech/Contacts.conf)
pkg_qtopiapim_settings.path=/etc/default/Trolltech
INSTALLS+=pkg_qtopiapim_settings

apics.files=$$QTOPIA_DEPOT_PATH/pics/addressbook/*
apics.path=/pics/addressbook
apics.hint=pics

dpics.files=$$QTOPIA_DEPOT_PATH/pics/datebook/*
dpics.path=/pics/datebook
dpics.hint=pics

tpics.files=$$QTOPIA_DEPOT_PATH/pics/todolist/*
tpics.path=/pics/todolist
tpics.hint=pics

INSTALLS+=apics dpics tpics

!qbuild{
sdk_qtopiapim_headers.files=$${HEADERS}
sdk_qtopiapim_headers.path=/include/qtopia/pim
sdk_qtopiapim_headers.hint=sdk headers
INSTALLS+=sdk_qtopiapim_headers

sdk_qtopiapim_private_headers.files=$${SEMI_PRIVATE_HEADERS}
sdk_qtopiapim_private_headers.path=/include/qtopia/pim/private
sdk_qtopiapim_private_headers.hint=sdk headers
INSTALLS+=sdk_qtopiapim_private_headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}

