qtopia_project(theme)

portalconf.files=../portal.conf
portalconf.path=/etc/themes
portalconf.trtarget=Portal
portalconf.hint=themecfg
portaldata.files=*.xml *rc
portaldata.path=/etc/themes/portal
portalpics.files=$$QTOPIA_DEPOT_PATH/pics/themes/portal/*
portalpics.path=/pics/themes/portal
portalpics.hint=pics

INSTALLS+=portalconf portaldata portalpics

portalbgimage.files=$$QTOPIA_DEPOT_PATH/pics/themes/portal/homebg.png
portalbgimage.path=/pics/themes/portal
portalbgimage.hint=background
# let this install first so we can overwrite the image
portalbgimage.depends=install_portalpics
INSTALLS+=portalbgimage

pkg.name=qpe-theme-portal
pkg.desc=Portal theme
pkg.domain=theme
