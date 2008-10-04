TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=qpredictivekeyboard

PLUGIN_FOR=qtopia
PLUGIN_TYPE=inputmethods

QTOPIA*=theming

include(predictivekeyboard.pro)

for(l,QTOPIA_LANGUAGES) {
    exists(/etc/default/Trolltech/$$l/PredictiveKeyboardLayout.conf) {
        eval(settings_$${l}.files=/etc/default/Trolltech/$$l/PredictiveKeyboardLayout.conf)
        eval(settings_$${l}.path=/etc/default/Trolltech/$$l)
        INSTALLS+=settings_$$l
    }
}
