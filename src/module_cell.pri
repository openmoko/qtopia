#This file contains projects that make up the Cell module.

#this module is dependent on telephony module
include(module_telephony.pri)

PROJECTS*=server/phone/telephony/phoneserver/atmodemservice \ #starts modem based telephony
          server/phone/telephony/phoneserver/dummyservice \   #support for QTOPIA_PHONE_DUMMY
          server/phone/telephony/callpolicymanager/cell \     #cell modem manager
          server/phone/telephony/simcallhistory \             #populate call history from SIM
          server/phone/telephony/cell/cellbroadcast \         #cellbroadcast management
          server/phone/telephony/dialfilter/gsm \             #GSM dial filter -> GCF compliance
          server/phone/telephony/cell/supplementaryservice \  #unstr. suppl service notifications
          server/phone/telephony/cell/simcontrolevent \       #handle simtoolkit control events
          server/phone/ui/components/simapp \                 #SIM toolkit application
          server/net/gprsmonitor                              #GPRS state (used by e.g. themes
      
PROJECTS*=\
        libraries/qtopiaphonemodem

qbuild{
    # Enable phonevendor and multiplexer plugins (selected via configure)
    for(p,PHONEVENDORS) {
        PROJECTS*=plugins/phonevendors/$$p
    }
    for(m,MULTIPLEXERS) {
        PROJECTS*=plugins/multiplexers/$$m
    }
} else {
    # Enable phonevendor and multiplexer plugins (selected via configure)
    for(p,PHONEVENDORS) {
        exists(plugins/phonevendors/$$p/$$tail($$p).pro):PROJECTS*=plugins/phonevendors/$$p
    }
    for(m,MULTIPLEXERS) {
        exists(plugins/multiplexers/$$m/$$tail($$m).pro):PROJECTS*=plugins/multiplexers/$$m
    }
}

PROJECTS*=\
        settings/callforwarding\
        settings/phonenetworks\
        settings/phonesettings

contains(PROJECTS,tools/atinterface) {
    PROJECTS*=libraries/qtopiamail
}
