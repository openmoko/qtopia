qbuild{
SOURCEPATH+=vpn
} else {
PREFIX=VPN
VPATH+=vpn
}

VPN_HEADERS+=\
    qvpnclient.h\
    qvpnfactory.h 

VPN_SOURCES+=\
    qvpnclient.cpp\
    qvpnfactory.cpp\
    qopenvpn.cpp\
    qopenvpngui.cpp\
    qipsec.cpp

VPN_PRIVATE_HEADERS+=\
    qvpnclientprivate_p.h\
    qopenvpn_p.h\
    qipsec_p.h

# Don't put this in include because it depends on the forms
VPN_PRIVATE_HEADERS+=\
    qopenvpngui_p.h

VPN_FORMS+=\
    generalopenvpnbase.ui\
    certificateopenvpnbase.ui\
    optionsopenvpnbase.ui\
    deviceopenvpnbase.ui

qbuild{
HEADERS+=$$VPN_HEADERS
SOURCES+=$$VPN_SOURCES
PRIVATE_HEADERS+=$$VPN_PRIVATE_HEADERS
FORMS+=$$VPN_FORMS
} else {
sdk_vpn_headers.files=$$VPN_HEADERS
sdk_vpn_headers.path=/include/qtopia/comm
sdk_vpn_headers.hint=sdk headers
INSTALLS+=sdk_vpn_headers

# the server does this for us
#vpn_categories.files=$$QTOPIA_DEPOT_PATH/etc/categories/vpn.conf
#vpn_categories.trtarget=QtopiaCategories
#vpn_categories.hint=nct
#INSTALLS+=vpn_categories
}
