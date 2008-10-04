!qbuild{
qtopia_project(theme)
}

deskphoneconf.files=home_wvga.conf
deskphoneconf.path=/etc/themes
deskphoneconf.trtarget=Theme-home_wvga
deskphoneconf.hint=themecfg
INSTALLS+=deskphoneconf

deskphonedata.files=../deskphone/*.xml *.xml *rc
deskphonedata.path=/etc/themes/deskphone
INSTALLS+=deskphonedata

deskphonepics.files=../deskphone/pics/*
deskphonepics.path=/pics/themes/deskphone
deskphonepics.hint=pics
INSTALLS+=deskphonepics

colors.files=home_wvga.scheme
colors.path=/etc/colors
INSTALLS+=colors

pkg.name=qpe-theme-deskphone-wvga
pkg.desc=Deskphone WVGA theme
pkg.domain=trusted
