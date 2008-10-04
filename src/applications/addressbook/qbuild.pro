TEMPLATE=app
CONFIG+=qtopia
TARGET=addressbook

QTOPIA*=pim mail collective
enable_telephony:QTOPIA*=phone
CONFIG+=quicklaunch singleexec
equals(QTOPIA_UI,home):MODULES*=homeui

include(addressbook.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

for(l,QTOPIA_LANGUAGES) {
    exists(/etc/default/Trolltech/$$l/AlphabetRibbonLayout.conf) {
        eval(settings_$${l}.files=/etc/default/Trolltech/$$l/AlphabetRibbonLayout.conf)
        eval(settings_$${l}.path=/etc/default/Trolltech/$$l)
        INSTALLS+=settings_$$l
    }
}

