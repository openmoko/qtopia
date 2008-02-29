PROJECTS*=\
    devtools\
    devtools/apm\
    devtools/qipkg-simple\
    devtools/chvol\
    devtools/startupflags\
    plugins/qtopiacore/kbddrivers/greenphone

enable_modem {
    PROJECTS*=\
        devtools/fixbdaddr
}

!free_package|free_plus_binaries {
    for(p,PHONEVENDORS) {
        exists(plugins/phonevendors/$$p/$$tail($$p).pro):PROJECTS*=plugins/phonevendors/$$p
    }
    for(m,MULTIPLEXERS) {
        exists(plugins/multiplexers/$$m/$$tail($$m).pro):PROJECTS*=plugins/multiplexers/$$m
    }
}
