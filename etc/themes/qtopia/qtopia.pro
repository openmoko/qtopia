qtopia_project(theme)

qtopiaconf.files=$$QTOPIA_DEPOT_PATH/etc/themes/qtopia.conf
qtopiaconf.path=/etc/themes
qtopiaconf.trtarget=Qtopia
qtopiaconf.hint=themecfg
qtopiaconf.outdir=$$PWD
qtopiadata.files=$$QTOPIA_DEPOT_PATH/etc/themes/qtopia/*.xml $$QTOPIA_DEPOT_PATH/etc/themes/qtopia/*rc
qtopiadata.path=/etc/themes/qtopia
qtopiapics.files=$$QTOPIA_DEPOT_PATH/pics/themes/qtopia
qtopiapics.path=/pics/themes
qtopiapics.hint=pics

INSTALLS+=qtopiaconf qtopiadata qtopiapics

qtopiabgimage.files=$$QTOPIA_DEPOT_PATH/pics/themes/qtopia/background.png
qtopiabgimage.path=/pics/themes/qtopia
qtopiabgimage.hint=background
# let this install first so we can overwrite the image
qtopiabgimage.depends=install_qtopiapics
INSTALLS+=qtopiabgimage

pkg.name=qpe-theme-qtopia
pkg.desc=Qtopia theme
pkg.domain=theme
