qtopia_project(theme)

gelconf.files=../gel.conf
gelconf.path=/etc/themes
gelconf.trtarget=Gel
gelconf.hint=themecfg
geldata.files=*.xml *rc
geldata.path=/etc/themes/gel
gelpics.files=$$QTOPIA_DEPOT_PATH/pics/themes/gel/*
gelpics.path=/pics/themes/gel
gelpics.hint=pics

INSTALLS+=gelconf geldata gelpics

gelbgimage.files=$$QTOPIA_DEPOT_PATH/pics/themes/gel/bground.png
gelbgimage.path=/pics/themes/gel
gelbgimage.hint=background
# let this install first so we can overwrite the image
gelbgimage.depends=install_gelpics
INSTALLS+=gelbgimage

pkg.name=qpe-theme-gel
pkg.desc=Gel theme
pkg.domain=theme
