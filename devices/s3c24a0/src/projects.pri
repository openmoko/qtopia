PROJECTS*=\
    plugins/qtopiacore/kbddrivers/s3c24a0\
    plugins/qtopiacore/mousedrivers/s3c24a0

enable_modem {
    for(p,PHONEVENDORS) {
        exists(plugins/phonevendors/$$p/$$tail($$p).pro):PROJECTS*=plugins/phonevendors/$$p
    }
    for(m,MULTIPLEXERS) {
        exists(plugins/multiplexers/$$m/$$tail($$m).pro):PROJECTS*=plugins/multiplexers/$$m
    }
}
