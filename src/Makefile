default:  dynamic

COMMERCIALLIBS=\
	libraries/qtopiamail

COMMERCIALCOMPONENTS=\
	plugins/today/email \
	plugins/applets/mountmon

COMMERCIALAPPS=\
	applications/qtmail \
	settings/backuprestore \
	settings/buttoneditor

LIBS3=$(LOCALLIBS) \
	libraries/qtopiapim \
	libraries/qtopiacalc \

LIBS2=\
	libraries/qtopia1

LIBS1=\
	libraries/qtopia \
	3rdparty/libraries/gsm \
	3rdparty/libraries/freetype \
	3rdparty/libraries/libavformat \
	3rdparty/libraries/libavcodec

$(LIBS2): $(LIBS1)

$(LIBS3): $(LIBS2)

LIBS=$(LIBS1) $(LIBS2) $(LIBS3)

COMPONENTS=$(LOCALCOMPONENTS) \
	3rdparty/plugins/codecs/libffmpeg \
	3rdparty/plugins/inputmethods/pickboard \
	3rdparty/plugins/obex/openobex \
	3rdparty/plugins/obex \
	3rdparty/plugins/applets/cardmon \
	3rdparty/plugins/textcodecs/jp \
	plugins/applets/batteryapplet \
	plugins/applets/brightness \
	plugins/applets/volumeapplet \
	plugins/applets/clockapplet \
	plugins/applets/clipboardapplet \
	plugins/applets/netmonapplet \
	plugins/applets/irreceiver \
	plugins/imagecodecs/notepad \
	plugins/codecs/wavplugin \
	plugins/codecs/wavrecord \
	plugins/textcodecs/simple8 \
	plugins/inputmethods/keyboard \
	plugins/inputmethods/handwriting \
	plugins/inputmethods/unikeyboard \
	plugins/calculator/simple \
	plugins/calculator/advanced \
	plugins/today/todo \
	plugins/today/datebook \
	plugins/network/dialup \
	plugins/network/lan \
	plugins/styles/fresh \
	plugins/styles/flat \
	plugins/decorations/polished \
	plugins/decorations/flat \
	plugins/fontfactories/freetype

APPS=$(LOCALAPPS) \
	3rdparty/applications/embeddedkonsole \
	3rdparty/applications/keypebble \
	3rdparty/tools/atd \
	applications/addressbook \
	applications/calculator \
	applications/clock \
	applications/datebook \
	applications/filebrowser \
	applications/helpbrowser \
	applications/imageviewer \
	applications/mediaplayer \
	applications/mediarecorder \
	applications/sysinfo \
	applications/textedit \
	applications/todo \
	applications/today \
	games/fifteen \
	games/go \
	games/mindbreaker \
	games/minesweep \
	games/parashoot \
	games/qasteroids \
	games/snake \
	games/solitaire \
	games/wordgame \
	settings/appearance \
	settings/appservices \
	settings/light-and-power \
	settings/language \
	settings/rotation \
	settings/security \
	settings/sound \
	settings/systemtime \
	settings/network \
	settings/qipkg \
	settings/launcher \
	settings/pluginmgr \
	tools/qcop \
	tools/quickexec \
	settings/worldtime

EXTRAPACKAGES=\
		$(QPEDIR)/src/qt/qpe-babelfish-font.control \
		$(QPEDIR)/src/qt/qt-embedded-jpeg.control \
		$(QPEDIR)/src/qt/qpe-utopia-font.control \
		$(QPEDIR)/src/qt/qt-embedded-fonts-t0.control \
		$(QPEDIR)/src/qt/qt-embedded-fonts-t5.control \
		$(QPEDIR)/src/qt/qt-embedded-fonts-t10.control \
		$(QPEDIR)/src/qt/qt-embedded-fonts-t15.control \
		$(QPEDIR)/src/qt/qt-embedded.control \
		$(QPEDIR)/src/qt/qpf-unifont-160-50-t10.control \
		$(QPEDIR)/pics/mediaplayer/skins/techno/qpe-mediaplayer-techno-skin.control \
		$(QPEDIR)/pics/qpe-pics.control \
		$(QPEDIR)/sounds/qpe-sounds.control \
		$(QPEDIR)/help/html/qpe-help-en.control \
		$(QPEDIR)/services/qpe-service-Calendar.control \
		$(QPEDIR)/services/qpe-service-Tasks.control \
		$(QPEDIR)/services/qpe-service-Contacts.control \
		$(QPEDIR)/services/qpe-service-Email.control \
		$(QPEDIR)/services/qpe-service-TimeMonitor.control \
		$(QPEDIR)/i18n/qpe-i18n-de.control \
		$(QPEDIR)/i18n/qpe-i18n-en.control \
		$(QPEDIR)/i18n/qpe-i18n-hu.control \
		$(QPEDIR)/i18n/qpe-i18n-ja.control \
		$(QPEDIR)/i18n/qpe-i18n-ko.control \
		$(QPEDIR)/i18n/qpe-i18n-no.control \
		$(QPEDIR)/i18n/qpe-i18n-zh-CN.control \
		$(QPEDIR)/i18n/qpe-i18n-zh-TW.control \
		$(LOCALEXTRAPACKAGES) \
	
dynamic: server $(APPS) $(COMPONENTS)

$(COMPONENTS): $(LIBS)

$(APPS): $(LIBS)

server: $(APPS)

$(LIBS) $(COMPONENTS) server $(APPS):
	$(MAKE) -C $@

showcomponents:
	@echo $(APPS) $(COMPONENTS) $(LIBS)

showdesktopcomponents:
	@echo libraries/qtopia libraries/qtopia1 libraries/qtopiapim

showwin32desktopcomponents:
	@echo libraries/qtopia libraries/qtopiapim

desktop: 
	$(MAKE) -C libraries/qtopia
	$(MAKE) -C libraries/qtopia1
	$(MAKE) -C libraries/qtopiapim

showwin32components:
	@echo $(LIBS)
	@echo $(APPS)
	@echo $(COMPONENTS)
	@echo server

showlibraries:
	@echo $(LIBS)

clean:
	for dir in server $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

lupdate:
	for dir in server $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

lrelease:
	for dir in server $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

packages:
	mkdir -p $(QPEDIR)/ipkg
	for dir in server $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir ipkg || exit 1; done
	for ctrl in $(EXTRAPACKAGES); do cd $(QPEDIR)/ipkg; ../bin/mkipks $$ctrl; done
	cd $(QPEDIR)/ipkg; ../scripts/mkPackages

.PHONY: default dynamic desktop server $(LIBS) $(APPS) $(COMPONENTS) showcomponents clean
