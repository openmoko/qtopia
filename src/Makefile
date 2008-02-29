default:  dynamic


COMMERCIALLIBS=\
	libraries/qtopiamail

COMMERCIALCOMPONENTS=\
	plugins/today/email \
	plugins/applets/mountmon

COMMERCIALAPPS=\
	applications/firstuse \
	applications/qtmail \
	settings/backuprestore \
	settings/buttoneditor

LIBS3=$(LOCALLIBS) \
	libraries/qtopiapim \
	libraries/qtopiaaccessory \
	libraries/qtopiacalc \

LIBS2=\
	libraries/qtopiaservices

LIBS1=\
	libraries/qtopia \
	3rdparty/libraries/freetype

$(LIBS2): $(LIBS1)

$(LIBS3): $(LIBS2)

LIBS=$(LIBS1) $(LIBS2) $(LIBS3)

COMPONENTS=$(LOCALCOMPONENTS) \
	3rdparty/plugins/codecs/libmpeg3 \
	3rdparty/plugins/inputmethods/pickboard \
	3rdparty/plugins/obex/openobex \
	3rdparty/plugins/obex \
	3rdparty/plugins/applets/cardmon \
	plugins/applets/batteryapplet \
	plugins/applets/brightness \
	plugins/applets/volumeapplet \
	plugins/applets/clockapplet \
	plugins/applets/clipboardapplet \
	plugins/applets/netmonapplet \
	plugins/applets/irreceiver \
	plugins/imagecodecs/notepad \
	plugins/codecs/wavplugin \
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
	plugins/styles/light \
	plugins/styles/flat \
	plugins/decorations/polished \
	plugins/decorations/flat \
	plugins/fontfactories/freetype

APPS=$(LOCALAPPS) \
	server \
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
	settings/taskbar \
	tools/qcop \
	tools/quickexec \
	settings/worldtime

EXTRAPACKAGES=\
		$(QPEDIR)/pics/mediaplayer/skins/techno/qpe-mediaplayer-techno-skin.control \
		$(QPEDIR)/pics/qpe-pics.control \
		$(QPEDIR)/sounds/qpe-sounds.control \
		$(QPEDIR)/help/en/qpe-help-en.control \
		$(QPEDIR)/services/qpe-service-Calendar.control \
		$(QPEDIR)/i18n/qpe-i18n-de.control \
		$(QPEDIR)/i18n/qpe-i18n-en.control \
		$(QPEDIR)/i18n/qpe-i18n-hu.control \
		$(QPEDIR)/i18n/qpe-i18n-ja.control \
		$(QPEDIR)/i18n/qpe-i18n-ko.control \
		$(QPEDIR)/i18n/qpe-i18n-no.control \
		$(QPEDIR)/i18n/qpe-i18n-zh-CN.control \
		$(QPEDIR)/i18n/qpe-i18n-zh-TW.control \
		$(LOCALEXTRAPACKAGES) \
	
single: mpegplayer/libmpeg3

dynamic: $(APPS)

$(COMPONENTS): $(LIBS)

$(APPS): $(LIBS) $(COMPONENTS)

$(LIBS) $(COMPONENTS) $(APPS) single:
	$(MAKE) -C $@

showcomponents:
	@echo qt $(LIBS) $(APPS) $(COMPONENTS) single

showlibraries:
	@echo $(LIBS)

clean:
	$(MAKE) -C single $@
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

lupdate:
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

lrelease:
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

packages:
	mkdir -p $(QPEDIR)/ipkg
	for dir in qt $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir ipkg || exit 1; done
	for ctrl in $(EXTRAPACKAGES); do cd $(QPEDIR)/ipkg; ../bin/mkipks $$ctrl; done
	cd $(QPEDIR)/ipkg; ../scripts/mkPackages

.PHONY: default dynamic $(LIBS) $(APPS) $(COMPONENTS) single showcomponents clean
