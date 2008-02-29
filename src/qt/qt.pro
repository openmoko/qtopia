TARGET		= qt
QMAKE_POST_LINK = @true
win32:QMAKE_POST_LINK = echo
QMAKE_LINK	= @true
win32:QMAKE_LINK = echo

lupdate.command_override=\
    for lang in $$TRANSLATIONS; do\
	$${DQTDIR}/bin/lupdate $${QTEDIR}/src/*/*.cpp -ts qt-\$$lang.ts;\
    done

qte_depot {
    QTESOURCE=$$(QPEDIR)
} else {
    QTESOURCE=$${QTEDIR}
}
!buildSingleexec {
    thread:libqte=libqte-mt
    !thread:libqte=libqte
    libqteinst.commands=$${COMMAND_HEADER}\
	install -c "$${QTESOURCE}/lib/$${libqte}.so.$${QTE_VERSION}" "$(INSTALL_ROOT)/lib/$${libqte}.so.$${QTE_VERSION}";\
	ln -sf $${libqte}.so.$${QTE_VERSION} $(INSTALL_ROOT)/lib/$${libqte}.so;\
	ln -sf $${libqte}.so.$${QTE_VERSION} $(INSTALL_ROOT)/lib/$${libqte}.so.$${QTE_MAJOR_VERSION};\
	ln -sf $${libqte}.so.$${QTE_VERSION} $(INSTALL_ROOT)/lib/$${libqte}.so.$${QTE_MAJOR_VERSION}.$${QTE_MINOR_VERSION};
    libqteinst.path=/lib

    buildQtopia:INSTALLS+=libqteinst
}

fontdir.files=$${QTEDIR}/lib/fonts/fontdir
fontdir.path=/lib/fonts
INSTALLS+=fontdir

PACKAGE_NAME=qt-embedded
PACKAGE_DESCRIPTION=Qt/Embedded
VERSION=$$QTE_VERSION
