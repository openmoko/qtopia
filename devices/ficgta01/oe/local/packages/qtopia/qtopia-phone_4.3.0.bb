DESCRIPTION = "Qtopia OpenSource"
SECTION = "qtopia-phone"
LICENSE = "GPL"
PRIORITY = "optional"
HOMEPAGE = "http://www.trolltech.com"
DEPENDS = "glib-2.0 dbus tslib gstreamer"
PROVIDES = "qtopia-phone"

#we can use "libpng libmng zlib jpeg"
PR = "r1"

########## DEVICE SPECIFIC - FIXME ##########

export RPREFIX=/opt/Qtopia

TARGET_DEVICE="${@base_contains('MACHINE', 'nokia770', ' nokia770', '',d)}"
TARGET_DEVICE="${@base_contains('MACHINE', 'nokia800', ' nokia770', '',d)}"
TARGET_DEVICE="${@base_contains('MACHINE', 'spitz', ' c3200', '',d)}"
TARGET_DEVICE="${@base_contains('MACHINE', 'tosa', ' c3200', '',d)}"
TARGET_DEVICE="${@base_contains('MACHINE', 'fic-gta01', 'ficgta01', '',d)}"


export DISPLAY_SIZE= "480x640"

########## DEVICE SPECIFIC - FIXME ##########

# special tech preview url

SRC_URI = "ftp://ftp.trolltech.com/qtopia/tech-preview/qtopia-opensource-src-${PV}-preview.tar.gz \
          file://linux-oe-qmake.conf"


S = "${WORKDIR}/qtopia-opensource-${PV}"
BUILD_DIR=${WORKDIR}/build

#PARALLEL_MAKE = ""

inherit pkgconfig update-rc.d

INITSCRIPT_NAME = "qpe"
INITSCRIPT_PARAMS = "defaults 98"

export QTOPIA_DEPOT_PATH = "${S}"

EXTRA_OEMAKE = " MAKEFLAGS= "


export OE_QMAKE_CC="${CC}"
export OE_QMAKE_CFLAGS="${CFLAGS}"
export OE_QMAKE_CXX="${CXX}"
export OE_QMAKE_CXXFLAGS="-fno-exceptions ${CXXFLAGS}"
export OE_QMAKE_LDFLAGS="${LDFLAGS}"
export OE_QMAKE_LINK="${CXX}"
export OE_QMAKE_AR="${AR}"
export OE_QMAKE_RANLIB="${RANLIB}"
export OE_QMAKE_STRIP="${STRIP}"
export OE_QMAKE_RPATH="-Wl,-rpath-link,"
export OE_QMAKE_INCDIR_QT="${QTDIR}/include"
export OE_QMAKE_LIBDIR_QT="${QTDIR}/lib"
export OE_QMAKE_INCDIR_QTOPIA="${QTOPIADIR}/include"
export OE_QMAKE_LIBDIR_QTOPIA="${QTOPIADIR}/lib"

require qte-functions.inc
QT_ARCH := "${@qte_arch(d)}"

do_unpack_extras() {
    
#    cd ${S}
#    cp -ar ${WORKDIR}/devices ${S}
#    cp -ar ${WORKDIR}/src ${S}
echo "nothing done at this time..."

}

addtask unpack_extras  before do_patch after do_unpack

do_patch() {
    BUILD_DIR=${WORKDIR}/build
	# Install the OE build templates
	
	# solve weirdnesses in Qmake
	sed -i "s@\$(OE_QMAKE_CC)@${CC}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_CFLAGS)@${CFLAGS}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(CFLAGS_EXTRA)@${CFLAGS_EXTRA}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_CXX)@${CXX}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_CXXFLAGS)@-fno-exceptions ${CXXFLAGS}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(CXXFLAGS_EXTRA)@${CXXFLAGS_EXTRA}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_LDFLAGS)@${LDFLAGS}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(LDFLAGS_EXTRA)@${LDFLAGS_EXTRA}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(LFLAGS_EXTRA)@${LFLAGS_EXTRA}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_LINK)@${CXX}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_AR)@${AR}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_RANLIB)@${RANLIB}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_STRIP)@${STRIP}@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_RPATH)@-Wl,-rpath-link,@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_INCDIR_QT)@${QTDIR}/include@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_LIBDIR_QT)@${QTDIR}/lib@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_INCDIR_QTOPIA)@${QTOPIADIR}/include@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@\$(OE_QMAKE_LIBDIR_QTOPIA)@${QTOPIADIR}/lib@g" ${WORKDIR}/linux-oe-qmake.conf

	sed -i "s@\$(COPY)@cp@g" ${WORKDIR}/linux-oe-qmake.conf
	sed -i "s@ccache @@g" ${WORKDIR}/linux-oe-qmake.conf

	for template in linux-oe-g++ linux-uclibc-oe-g++ linux-gnueabi-oe-g++ linux-gnueabi-g++
	do
    install -d ${S}/qtopiacore/qt/mkspecs/$template
    install -m 0644 ${WORKDIR}/linux-oe-qmake.conf \
      ${S}/qtopiacore/qt/mkspecs/$template/qmake.conf
    ln -sf ${S}/qtopiacore/qt/mkspecs/linux-g++/qplatformdefs.h \
      ${S}/qtopiacore/qt/mkspecs/$template/qplatformdefs.h

		install -d ${S}/devices/${TARGET_DEVICE}/mkspecs/qws/$template
		install -m 0644 ${WORKDIR}/linux-oe-qmake.conf \
			${S}/devices/${TARGET_DEVICE}/mkspecs/qws/$template/qmake.conf
		ln -sf ${S}/devices/${TARGET_DEVICE}/mkspecs/qws/linux-${TARGET_DEVICE}-g++/qplatformdefs.h \
			${S}/devices/${TARGET_DEVICE}/mkspecs/qws/$template/qplatformdefs.h
	done
  
}

do_configure() {

BUILD_DIR=${WORKDIR}/build
mkdir -p $BUILD_DIR
	unset CC
	unset CXX
	unset CFLAGS
	unset CXXFLAGS
	unset LDFLAGS

  chmod 664 -v ${S}/devices/${TARGET_DEVICE}/configure
    
##################
# might want to pick and choose here

# EXTRA_CONFIG+=="\"-helix -helix-system-id linux-2.2-libc6-xscale-cross-gcc32\""
# -asterisk
# -voip
# -ssl
# -profile
# -singleexec
# -displayrot "0,90,180,270"
# -iconsize 14,16,22
DEPTHS="16"
# 8,16,32

EXTRA_CONFIG+=" -qtopiamedia "
EXTRA_CONFIG+=" -mediaengines gstreamer"

#EXTRA_CONFIG+=" -mediaengines helix"
#EXTRA_CONFIG+=" -helix-system-id linux-2.2-libc6-arm9-cross-gcc4"


EXTRA_CONFIG+=" -extra-qtopiacore-config \"-qt-mouse-linuxtp\" "
EXTRA_CONFIG+=" -extra-qtopiacore-config \"-depths $DEPTHS\" "

#EXTRA_CONFIG+=" -iconsize 22"
#EXTRA_CONFIG+=" -sound-system alsa"
#EXTRA_CONFIG+=" -bluetooth"
#EXTRA_CONFIG+=" -voip" 
#EXTRA_CONFIG+=" -no-modem"
#EXTRA_CONFIG+=" -force-quicklaunch"
#EXTRA_CONFIG+=" -no-ssl"
#EXTRA_CONFIG+=" -no-helix"
#EXTRA_CONFIG+=" "

EXTRA_CONFIG+="${@base_contains('MACHINE_FEATURES', 'alsa', ' -sound-system alsa', '',d)}"
EXTRA_CONFIG+="${@base_contains('MACHINE_FEATURES', 'bluetooth', ' -bluetooth', '',d)}"
EXTRA_CONFIG+="${@base_contains('MACHINE_FEATURES', 'wifi', ' -voip', '',d)}"
EXTRA_CONFIG+="${@base_contains('MACHINE_FEATURES', 'irda', ' -irda', '',d)}"
EXTRA_CONFIG+="${@base_contains('MACHINE_FEATURES', 'phone', ' -modem', '',d)}"


EXTRA_CONFIG+=" -no-infrared"
# EXTRA_CONFIG+=" -singleexec"
EXTRA_CONFIG+=" -dbus"
EXTRA_CONFIG+=" -dbuspath \"/opt/toolchains/arm920t-eabi/arm-angstrom-linux-gnueabi\""


# hidden configure arguments, can be used when already have uicmoc,
# qtx11 (linguist, etc) and qtopia core compiled
# EXTRA_CONFIG+=" -skip-dqt"
# EXTRA_CONFIG+=" -skip-qt"
# EXTRA_CONFIG+=" -skip-qte"

case ${GUI_MACHINE_CLASS} in
	bigscreen ) EXTRA_CONFIG+=" -iconsize 22" ;;
	smallscreen ) EXTRA_CONFIG+=" -iconsize 14" ;;
esac



XPLATFORM=linux-gnueabi-g++

ARCH=arm
EXTRA_DEPENDS="-D QT_OPEN_LARGEFILE -D SYSINFO_GEEK_MODE"
SOUND_SYSTEM="alsa"
FONTS=""

#-L${S}/lib
#-L${S}/qtopiacore/target/lib
#-I${S}/include
#-I${S}/qtopiacore/target/include
#-extra-qtopiacore-config \"-I${STAGING_INCDIR}/dbus-1.0\" 
#-I${STAGING_INCDIR} 
#-L${STAGING_LIBDIR}
# -extra-qtopiacore-config \"-plugin-gfx-transformed\" 

# -x11 -lSM -lICE

echo "-arch $ARCH 
-dbus 
-dbuspath ${STAGING_LIBDIR}/..
-displaysize $DISPLAY_SIZE
-L${S}/lib
-L${S}/qtopiacore/target/lib
-I${S}/include
-I${S}/qtopiacore/target/include
-extra-qtopiacore-config \"-D QT_QWS_SCREENCORDS\" 
-extra-qtopiacore-config \"-D QT_QWS_SCREEN_COORDINATES\"
-extra-qtopiacore-config \"-fast\" 
-extra-qtopiacore-config \"-little-endian\" 
-extra-qtopiacore-config \"-no-accessibility\" 
-extra-qtopiacore-config \"-no-cups\"
-extra-qtopiacore-config \"-no-freetype\" 
-extra-qtopiacore-config \"-nomake demos\" 
-extra-qtopiacore-config \"-nomake examples\"
-extra-qtopiacore-config \"-qt-gfx-linuxfb\" 
-extra-qtopiacore-config \"-qt-libjpeg\" 
-extra-qtopiacore-config \"-qt-libmng\" 
-extra-qtopiacore-config \"-qt-libpng\" 
-extra-qtopiacore-config \"-qt-mouse-tslib\" 
-extra-qtopiacore-config \"-qt-zlib\" 
-extra-qtopiacore-config \"-release\" 
-extra-qtopiacore-config \"-xplatform $XPLATFORM\"
-extra-qtopiacore-config \"-L${STAGING_LIBDIR}\" 
-extra-qtopiacore-config \"-I${STAGING_INCDIR}\" 
-extra-qtopiacore-config \"-L${S}/lib\" 
-extra-qtopiacore-config \"-L${S}/qtopiacore/target/lib\" 
-extra-qtopiacore-config \"-I${S}/include\" 
-extra-qtopiacore-config \"-I${S}/qtopiacore/target/include\" 
-extra-qt-config \"-nomake examples\" 
-extra-qt-config \"-nomake demos\" 
-font dejavu_sans_condensed:10,11,13,15,16,17,18,19,28,36:50,75
-image $RPREFIX
-launch-method quicklaunch
-force-quicklaunch
-no-clean 
-no-drm
-no-qvfb
-no-vpn
-no-sxe
-no-silent
-prefix $RPREFIX
-reduce-exports=yes
-release
-verbose
-confirm-license
-xplatform $XPLATFORM
$EXTRA_DEPENDS
$EXTRA_CONFIG
" > ${S}/devices/${TARGET_DEVICE}/configure


# mkdir $BUILD_DIR
  cd ${BUILD_DIR}
	echo yes | ${S}/configure -device ${TARGET_DEVICE}

}

do_compile() {
  cd ${WORKDIR}/build
	oe_runmake
}

STAGE_TEMP = "${WORKDIR}/temp-staging"
do_stage() {

  cd ${WORKDIR}/build
	rm -rf ${STAGE_TEMP}
	mkdir -p ${STAGE_TEMP}
	oe_runmake install INSTALL_ROOT=${STAGE_TEMP} IMAGE=${STAGE_TEMP}

	rm -rf ${STAGE_TEMP}
    
# ln -sf
## 	install -m 0755 ${BUILD_DIR}/qtopiacore/host/bin/rcc ${STAGING_BINDIR}
## 	install -m 0755 ${BUILD_DIR}/qtopiacore/host/bin/moc ${STAGING_BINDIR}
## 	install -m 0755 ${BUILD_DIR}/qtopiacore/host/bin/uic ${STAGING_BINDIR}
## 	install -m 0755 ${BUILD_DIR}/qtopiacore/host/bin/qmake ${STAGING_BINDIR}
## 	install -m 0755 ${BUILD_DIR}/qtopiacore/host/bin/lupdate ${STAGING_BINDIR}
## 	install -m 0755 ${BUILD_DIR}/qtopiacore/host/bin/lrelease ${STAGING_BINDIR}

## 	install -m 0755 ${S}/bin/qtopiamake ${STAGING_BINDIR}

## 	cp -af ${S}/qtopiacore/qt/mkspecs ${STAGING_DIR}
}


do_install() {
	cd ${WORKDIR}/build
	oe_runmake install INSTALL_ROOT=${D}${RPREFIX} IMAGE=${D}${RPREFIX}
 # uncomment these to create qtopia ipkg's
 # oe_runmake packages FORMAT=ipkg
 # cp ${WORKDIR}/build/pkg/* 
  install -d ${D}${sysconfdir}/init.d
  install -m 0755 ${S}/devices/${TARGET_DEVICE}/src/devtools/startup/qpe.sh ${D}${sysconfdir}/init.d/qpe

  chmod 755 ${D}${sysconfdir}/init.d/qpe

}


#QTOPIA_CORE_PACKAGES = "libqtcore4 libqtgui4 libqtnetwork4  \
#             libqtscripts4 libqtsql4 libqtsvg4 libqttest4 \
#             libqtxml4 "
#QTOPIA_3RDPARTY_LIBRARY_PACKAGES = "libamr libdissipate2 libgsm libinputmatch libmd5 libopenobex \
#libqtdbus libqtopia-sqlite \
#"
#QTOPIA_LIBRARY_PACKAGES = "libqtopia4 libqtopiaaudio4 libqtopiabase4 libqtopiacomm libqtopiamail \
#libqtopiamedia4 libqtopiaphone4 libqtopiaphonemodem4 libqtopiapim4 libqtopiaprinting4 \
#libqtopiasecurity4 libqtopiasmil4 \
#"
#QTOPIA_APPLICATION_PACKAGES = ""
#QTOPIA_GAMES_PACKAGES = ""
#QTOPIA_SETTINGS_PACKAGES = ""
#QTOPIA_PLUGINS = ""

FILES_${PN} += "${RPREFIX}/*"
FILES_${PN} +=  ${D}${sysconfdir}/init.d/qpe

# PACKAGES += "qtopia-phone"

#FILES_libqtcore4                   = "${libdir}/libQtCore.so.*"
#FILES_libqtcore4-dev               = "${libdir}/libQtCore.so"
#FILES_libqtgui4                    = "${libdir}/libQtGui.so.*"
#FILES_libqtgui4-dev                = "${libdir}/libQtGui.so"
#FILES_libqtnetwork4                = "${libdir}/libQtNetwork.so.*"
#FILES_libqtnetwork4-dev            = "${libdir}/libQtNetwork.so"
#FILES_libqtsql4                    = "${libdir}/libQtSql.so.*"
#FILES_libqtsql4-dev                = "${libdir}/libQtSql.so"
#FILES_libqtsvg4                    = "${libdir}/libQtSvg.so.*"
#FILES_libqtsvg4-dev                = "${libdir}/libQtSvg.so"
#FILES_libqttest4                   = "${libdir}/libQtTest.so.*"
#FILES_libqttest4-dev               = "${libdir}/libQtTest.so"
#FILES_libqtxml4                    = "${libdir}/libQtXml.so.*"
#FILES_libqtxml4-dev                = "${libdir}/libQtXml.so"
#FILES_libqtscript4                 = "${libdir}/libQtScript.so.*"
#FILES_libqtscript4-dev             = "${libdir}/libQtScript.so"
#FILES_libqtdbus4                   = "${libdir}/libQtDBus.so.*"
#FILES_libqtdbus4-dev               = "${libdir}/libQtDBus.so"

#FILES_qt4-plugins-accessible   = "${libdir}/plugins/accessible/*.so"
#FILES_qt4-plugins-codecs       = "${libdir}/plugins/codecs/*.so"
#FILES_qt4-plugins-imageformats = "${libdir}/plugins/imageformats/*.so"
##FILES_qt4-plugins-inputmethods = "${libdir}/plugins/inputmethods/*.so"
#FILES_qt4-plugins-iconengines  = "${libdir}/plugins/iconengines/*.so"

#FILES_qt4-dbus                 = "${bindir}/qdbus ${bindir}/qdbusxml2cpp ${bindir}/qdbuscpp2xml ${bindir}/qdbusviewer"

