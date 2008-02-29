HEADERS+=addressbook.h
SOURCES+=addressbook.cpp main.cpp

include (addressbook.pri)

CONFIG+=pimlib

service.files=$${QTOPIA_DEPOT_PATH}/services/Contacts/addressbook
service.path=/services/Contacts
receiveservice.files=$${QTOPIA_DEPOT_PATH}/services/Receive/text/x-vCard/addressbook
receiveservice.path=/services/Receive/text/x-vCard/
smsservice1.files=$${QTOPIA_DEPOT_PATH}/services/smsapp/226/addressbook
smsservice1.path=/services/smsapp/226/
smsservice2.files=$${QTOPIA_DEPOT_PATH}/services/smsapp/9204/addressbook
smsservice2.path=/services/smsapp/9204/
desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/addressbook.desktop
desktop.path=/apps/Applications
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=addressbook*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/addressbook/*
pics.path=/pics/addressbook
im.files=named_addressbook-*.conf
im.path=/etc/im/pkim
INSTALLS+=service desktop receiveservice smsservice1 smsservice2 im
PICS_INSTALLS+=pics
HELP_INSTALLS+=help
phoneservice.files=$${QTOPIA_DEPOT_PATH}/services/ContactsPhone/addressbook
phoneservice.path=/services/ContactsPhone
QTOPIA_PHONE:setvcardservice.files=$${QTOPIA_DEPOT_PATH}/services/SetValue/v-card/phone/addressbook
else:setvcardservice.files=$${QTOPIA_DEPOT_PATH}/services/SetValue/v-card/addressbook
setvcardservice.path=/services/SetValue/v-card/
qdlservice.files=$${QTOPIA_DEPOT_PATH}/services/qdl/addressbook
qdlservice.path=/services/qdl
INSTALLS+=setvcardservice phoneservice qdlservice

PACKAGE_DESCRIPTION=A simple addressbook for the Qtopia environment.
PACKAGE_DEPENDS+=qpe-libqtopiapim1
