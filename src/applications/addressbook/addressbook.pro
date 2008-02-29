HEADERS+=addressbook.h
SOURCES+=addressbook.cpp main.cpp

include (addressbook.pri)

CONFIG+=pimlib

service.files=$${QTOPIA_DEPOT_PATH}/services/Contacts/addressbook
service.path=/services/Contacts
receiveservice.files=$${QTOPIA_DEPOT_PATH}/services/Receive/text/x-vCard/addressbook
receiveservice.path=/services/Receive/text/x-vCard/
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/addressbook.desktop
desktop.path=/apps/Applications
help.files=$${QTOPIA_DEPOT_PATH}/help/html/addressbook*
help.path=/help/html
pics.files=$${QTOPIA_DEPOT_PATH}/pics/addressbook/*
pics.path=/pics/addressbook
im.files=named_addressbook-*.conf
im.path=/etc/im/pkim
INSTALLS+=service desktop receiveservice help im
PICS_INSTALLS+=pics
phoneservice.files=$${QTOPIA_DEPOT_PATH}/services/ContactsPhone/addressbook
phoneservice.path=/services/ContactsPhone
setvcardservice.files=$${QTOPIA_DEPOT_PATH}/services/SetValue/v-card/addressbook
setvcardservice.path=/services/SetValue/v-card/
qdlservice.files=$${QTOPIA_DEPOT_PATH}/services/qdl/addressbook
qdlservice.path=/services/qdl
INSTALLS+=setvcardservice phoneservice qdlservice

PACKAGE_DESCRIPTION=A simple addressbook for the Qtopia environment.
PACKAGE_DEPENDS+=qpe-libqtopiapim1
