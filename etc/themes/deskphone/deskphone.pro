!qbuild{
qtopia_project(theme)
}

deskphoneconf.files=deskphone.conf
deskphoneconf.path=/etc/themes
deskphoneconf.trtarget=Theme-Deskphone
deskphoneconf.hint=themecfg
INSTALLS+=deskphoneconf

deskphonedata.files=*.xml *rc
deskphonedata.path=/etc/themes/deskphone
INSTALLS+=deskphonedata

deskphonepics.files=pics/*
deskphonepics.path=/pics/themes/deskphone
deskphonepics.hint=pics
INSTALLS+=deskphonepics

colors.files=Deskphone.scheme
colors.path=/etc/colors
INSTALLS+=colors

pkg.name=qpe-theme-deskphone
pkg.desc=Deskphone theme
pkg.domain=trusted
