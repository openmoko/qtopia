default: dynamic

#Sections are
#
# Core (Used in/by server)
# Supported ( Known high quality )
# Unsupported ( Unknown quality )
# Commercial ( Only available for certain licensces )
# 
# Use a wrapping Makefile that defines LOCALCOMPONENTS/APPS/LIBS to include extra targets.

#Core/Misc
CORELIBS1=\
	libraries/qtopia \
	3rdparty/libraries/freetype

CORELIBS2=\
	libraries/qtopia1

# this depenedency is needed due to ui files in libraries/qtopia
$(CORELIBS2): $(CORELIBS1)

CORELIBS=\
	$(CORELIBS1) \
	$(CORELIBS2)

CORECOMPONENTS=\
	plugins/inputmethods/keyboard \
	plugins/inputmethods/handwriting \
	plugins/inputmethods/unikeyboard \
	3rdparty/plugins/obex/openobex \
	3rdparty/plugins/obex \
	plugins/applets/irreceiver \
	plugins/fontfactories/freetype

COREAPPS=server

$(CORECOMPONENTS): $(CORELIBS)

#Supported
SUPPORTEDLIBS=\
	libraries/qtopiapim \
	libraries/qtopiacalc \
	3rdparty/libraries/gsm \
	3rdparty/libraries/libavformat \
	3rdparty/libraries/libavcodec


SUPPORTEDCOMPONENTS=\
	3rdparty/plugins/codecs/libffmpeg \
	3rdparty/plugins/inputmethods/pickboard \
	3rdparty/plugins/applets/cardmon \
	3rdparty/plugins/textcodecs/jp \
	plugins/applets/batteryapplet \
	plugins/applets/brightness \
	plugins/applets/volumeapplet \
	plugins/applets/clockapplet \
	plugins/applets/clipboardapplet \
	plugins/applets/netmonapplet \
	plugins/imagecodecs/notepad \
	plugins/codecs/wavplugin \
	plugins/codecs/wavrecord \
	plugins/textcodecs/simple8 \
	plugins/calculator/simple \
	plugins/calculator/advanced \
	plugins/today/todo \
	plugins/today/datebook \
	plugins/network/dialup \
	plugins/network/lan \
	plugins/styles/fresh \
	plugins/styles/flat \
	plugins/decorations/polished \
	plugins/decorations/flat

SUPPORTEDAPPS=\
	3rdparty/tools/atd \
	applications/addressbook \
	applications/calculator \
	applications/clock \
	applications/datebook \
	applications/helpbrowser \
	applications/imageviewer \
	applications/mediaplayer \
	applications/mediarecorder \
	applications/sysinfo \
	applications/textedit \
	applications/todo \
	applications/today \
	games/fifteen \
	games/minesweep \
	games/wordgame \
	games/mindbreaker \
	games/parashoot \
	games/qasteroids \
	games/snake \
	games/solitaire \
	settings/appearance \
	settings/appservices \
	settings/light-and-power \
	settings/rotation \
	settings/security \
	settings/sound \
	settings/systemtime \
	settings/network \
	settings/qipkg \
	settings/pluginmgr \
	settings/worldtime \
	settings/launcher \
	settings/language \
	tools/quicklauncher \
	tools/qcop \
	tools/symlinker

$(SUPPORTEDLIBS): $(CORELIBS)

$(SUPPORTEDAPPS): $(SUPPORTEDLIBS) $(CORELIBS)

$(SUPPORTEDCOMPONENTS): $(SUPPORTEDLIBS) $(CORELIBS)

#Unsupported

UNSUPPORTEDAPPS=\
	applications/filebrowser \
	3rdparty/applications/embeddedkonsole \
	3rdparty/applications/keypebble

$(UNSUPPORTEDAPPS): $(CORELIBS) $(SUPPORTEDLIBS)

#Commercial
COMMERCIALLIBS=\
	libraries/qtopiamail

COMMERCIALCOMPONENTS=\
	plugins/today/email \
	plugins/applets/mountmon

COMMERCIALAPPS=\
	applications/qtmail \
	settings/backuprestore \
	settings/buttoneditor

$(COMMERCIALLIBS): $(SUPPORTEDLIBS) $(CORELIBS)

$(COMMERCIALAPPS): $(COMMERCIALLIBS) $(CORELIBS) $(SUPPORTEDLIBS)

$(COMMERCIALCOMPONENTS): $(COMMERCIALLIBS) $(CORELIBS)

#Packages, may later split as above.
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
		$(QPEDIR)/src/qt/qpf-japanese-230-50-t0.control \
		$(QPEDIR)/src/qt/qpf-japanese-230-50-t5.control \
		$(QPEDIR)/src/qt/qpf-japanese-230-50-t10.control \
		$(QPEDIR)/src/qt/qpf-japanese-230-50-t15.control \
		$(QPEDIR)/pics/mediaplayer/skins/techno/qpe-mediaplayer-techno-skin.control \
		$(QPEDIR)/pics/qpe-pics.control \
		$(QPEDIR)/sounds/qpe-sounds.control \
		$(QPEDIR)/help/html/qpe-help-en.control \
		$(QPEDIR)/services/qpe-service-Calendar.control \
		$(QPEDIR)/services/qpe-service-Tasks.control \
		$(QPEDIR)/services/qpe-service-Contacts.control \
		$(QPEDIR)/services/qpe-service-Email.control \
		$(QPEDIR)/services/qpe-service-TimeMonitor.control \
		$(QPEDIR)/i18n/qpe-i18n-en_GB.control \
		$(QPEDIR)/i18n/qpe-i18n-de.control \
		$(QPEDIR)/i18n/qpe-i18n-en.control \
		$(QPEDIR)/i18n/qpe-i18n-hu.control \
		$(QPEDIR)/i18n/qpe-i18n-ja.control \
		$(QPEDIR)/i18n/qpe-i18n-ko.control \
		$(QPEDIR)/i18n/qpe-i18n-no.control \
		$(QPEDIR)/i18n/qpe-i18n-zh-CN.control \
		$(QPEDIR)/i18n/qpe-i18n-zh-TW.control \
		$(LOCALEXTRAPACKAGES) \
	

# Deps for locals, (prepended)
$(LOCALLIBS): $(SUPPORTEDLIBS) $(CORELIBS)

$(LOCALAPPS): $(LOCALLIBS) $(CORELIBS) $(SUPPORTEDLIBS)

$(LOCALCOMPONENTS): $(LOCALLIBS) $(CORELIBS)

#Targets.
# These three are to aid build rules, not grouping or config.
LIBS=$(CORELIBS) \
	$(LOCALLIBS) \
	$(SUPPORTEDLIBS) \
	$(UNSUPPORTEDLIBS)

COMPONENTS=$(CORECOMPONENTS) \
	$(LOCALCOMPONENTS) \
	$(SUPPORTEDCOMPONENTS) \
	$(UNSUPPORTEDCOMPONENTS)

APPS=$(COREAPPS) \
	$(LOCALAPPS) \
	$(SUPPORTEDAPPS) \
	$(UNSUPPORTEDAPPS)


$(LIBS) $(COMPONENTS) $(APPS):
	$(MAKE) -C $@

$(COREAPPS): $(SUPPORTEDAPPS) $(SUPPORTEDCOMPONENTS)

#Targets.
dynamic: $(COREAPPS) \
	$(CORECOMPONENTS) \
	$(SUPPORTEDAPPS) \
	$(SUPPORTEDCOMPONENTS) \
	$(UNSUPPORTEDAPPS) \
	$(LOCALAPPS) \
	$(LOCALCOMPONENTS)

unsupported: $(UNSUPPORTEDAPPS) $(UNSUPPORTEDCOMPONENTS)

libraries: $(CORELIBS) \
	$(SUPPORTEDLIBS) \
	$(LOCALLIBS)

components: $(CORECOMPONENTS) \
	$(SUPPORTEDCOMPONENTS) \
	$(LOCALCOMPONENTS)

desktop: 
	$(MAKE) -C libraries/qtopia
	$(MAKE) -C libraries/qtopia1
	$(MAKE) -C libraries/qtopiapim
	$(MAKE) -C settings/worldtime worldtimebase.h worldtimebase.cpp
	$(MAKE) -C qtopiadesktop

showcomponents:
	@echo $(APPS) $(COMPONENTS) $(LIBS)

showdesktopcomponents:
	@echo libraries/qtopia libraries/qtopia1 libraries/qtopiapim settings/worldtime

showwin32desktopcomponents:
	@echo libraries/qtopia libraries/qtopiapim

showwin32components:
	@echo $(LIBS)
	@echo $(APPS)
	@echo $(COMPONENTS)

showlibraries:
	@echo $(LIBS)

#libs, server and plugins only
showlinksource:
	@echo server
	@echo $(CORELIBS)
	@echo $(CORECOMPONENTS)
	@echo $(SUPPORTEDLIBS)
	@echo $(SUPPORTEDCOMPONENTS)
	@echo $(COMMERCIALLIBS)
	@echo $(COMMERCIALCOMPONENTS)

clean:
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

# quiet, except for out-of-date files
lupdate-check:
	@for dir in $(APPS) $(LIBS) $(COMPONENTS); \
	do \
	    O=$$( $(MAKE) 2>&1 -C $$dir $@ | grep -v '^make.*directory'; ); \
	    if [ -n "$$O" ]; \
	    then \
		$(MAKE) -C $$dir $@; \
	    fi; \
	done

lupdate:
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

lrelease:
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir $@ || exit 1; done

packages:
	mkdir -p $(QPEDIR)/ipkg
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir ipkg || exit 1; done
	for ctrl in $(EXTRAPACKAGES); do cd $(QPEDIR)/ipkg; ../bin/mkipks $$ctrl; done
	cd $(QPEDIR)/ipkg; ../bin/mkPackages

.PHONY: default dynamic desktop libraries components $(LIBS) $(APPS) $(COMPONENTS) showcomponents showlinksource clean
