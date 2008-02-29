qtopia_project(theme)

crispconf.files=$$QTOPIA_DEPOT_PATH/etc/themes/crisp.conf
crispconf.path=/etc/themes
crispconf.trtarget=Crisp
crispconf.hint=themecfg
crispconf.outdir=$$PWD
crispdata.files=$$QTOPIA_DEPOT_PATH/etc/themes/crisp/*.xml $$QTOPIA_DEPOT_PATH/etc/themes/crisp/*rc
crispdata.path=/etc/themes/crisp
crisppics.files=$$QTOPIA_DEPOT_PATH/pics/themes/crisp/*
crisppics.path=/pics/themes/crisp
crisppics.hint=pics

INSTALLS+=crispconf crispdata crisppics

pkg.name=qpe-theme-crisp
pkg.desc=Crisp theme
pkg.domain=theme
