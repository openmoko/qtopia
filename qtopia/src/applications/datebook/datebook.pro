include (datebook.pri)

CONFIG+=pimlib

HEADERS+=datebookgui.h
SOURCES+=datebookgui.cpp\
         main.cpp

TRANSLATABLES += datebookgui.h \
                    datebookgui.cpp \
                    main.cpp

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/datebook.desktop
desktop.path=/apps/Applications
calservice.files=$${QTOPIA_DEPOT_PATH}/services/Calendar/datebook
calservice.path=/services/Calendar
timeservice.files=$${QTOPIA_DEPOT_PATH}/services/TimeMonitor/datebook
timeservice.path=/services/TimeMonitor
recservice.files=$${QTOPIA_DEPOT_PATH}/services/Receive/text/x-vCalendar-Events/datebook
recservice.path=/services/Receive/text/x-vCalendar-Events
qdlservice.files=$${QTOPIA_DEPOT_PATH}/services/qdl/datebook
qdlservice.path=/services/qdl
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=datebook*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/datebook/*
pics.path=/pics/datebook
INSTALLS+=desktop calservice timeservice recservice qdlservice
HELP_INSTALLS+=help
PICS_INSTALLS+=pics
 
PACKAGE_DESCRIPTION=Qtopia datebook/appointment manager.
PACKAGE_DEPENDS+=qpe-libqtopiapim1
