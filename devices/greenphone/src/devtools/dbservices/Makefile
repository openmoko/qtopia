define RULE
@+QPEDIR=$(QTOPIA_BUILD_TREE);\
if [ -z "$$QPEDIR" ]; then\
    QPEDIR=$(QPEDIR);\
fi;\
if [ -z "$$QPEDIR" ]; then\
    echo "ERROR: You must set QTOPIA_BUILD_TREE if you want to use depot hopping.";\
    echo "When this file was created, the Qtopia build tree was:";\
    echo "    /home/ssmith/build/qtopia/42-nosxe";\
    exit 1;\
fi;\
QPEDIR=`cd $$QPEDIR; /bin/pwd | sed 's/\/$$//'`;\
shadowdir=`echo $$PWD | perl -ne 's,/home/ssmith/depot/qtopia/4.2,'$$QPEDIR',;\
                                    s,/src/libraries/([^\d]+)\d?,/src/libraries/$$1,;\
                                    print'`;\
if [ "$$shadowdir" != "$$PWD" -a -d "$$shadowdir" ]; then\
    cd "$$shadowdir";\
    $(MAKE) $@;\
else\
    echo "ERROR: Can't resolve shadowdir.";\
    echo "       SOURCE tree = '/home/ssmith/depot/qtopia/4.2'";\
    echo "       BUILD  tree = '$$QPEDIR'";\
    echo "       PWD         = '$$PWD'";\
    echo "       shadowdir   = '$$shadowdir'";\
    exit 1;\
fi
endef

all:
	$(RULE)
install:
	$(RULE)
uninstall:
	$(RULE)
install_target:
	$(RULE)
packages:
	$(RULE)
lupdate:
	$(RULE)
sdk:
	$(RULE)
cleaninstall:
	$(RULE)
cinstall:
	$(RULE)
syncqtopia:
	$(RULE)
first_syncqtopia:
	$(RULE)
test:
	$(RULE)
checkp4:
	$(RULE)
remove_target:
	$(RULE)
relink:
	$(RULE)
printdependrules:
	$(RULE)
qmake_check:
	$(RULE)
qtopia_first:
	$(RULE)
qtopia_all:
	$(RULE)
qtopia_clean:
	$(RULE)
qtopia_distclean:
	$(RULE)
qtopia_install:
	$(RULE)
qtopia_uninstall:
	$(RULE)
first:
	$(RULE)
dist:
	$(RULE)
yaccclean:
	$(RULE)
lexclean:
	$(RULE)
clean:
	$(RULE)
distclean:
	$(RULE)
mocclean:
	$(RULE)
mocables:
	$(RULE)
package_qpe-dbservices:
	$(RULE)
compiler_moc_header_make_all:
	$(RULE)
compiler_moc_header_clean:
	$(RULE)
compiler_rcc_make_all:
	$(RULE)
compiler_rcc_clean:
	$(RULE)
compiler_image_collection_make_all:
	$(RULE)
compiler_image_collection_clean:
	$(RULE)
compiler_moc_source_make_all:
	$(RULE)
compiler_moc_source_clean:
	$(RULE)
compiler_uic_make_all:
	$(RULE)
compiler_uic_clean:
	$(RULE)
compiler_translatables_make_all:
	$(RULE)
dummy_translatables:
	$(RULE)
compiler_runcheckp4_make_all:
	$(RULE)
dummy_checkp4:
	$(RULE)
compiler_clean:
	$(RULE)
install_service:
	$(RULE)
uninstall_service:
	$(RULE)
install_linstall:
	$(RULE)
uninstall_target:
	$(RULE)
install_help:
	$(RULE)
install_docapi_desktop:
	$(RULE)
install_pics:
	$(RULE)
qmake:
	$(RULE)
qmake-debug:
	$(RULE)
regenerate:
	$(RULE)
regenerate_all:
	$(RULE)
nodeps:
	$(RULE)
loud:
	$(RULE)
silent:
	$(RULE)
.PHONY: all install uninstall install_target packages lupdate sdk cleaninstall cinstall syncqtopia first_syncqtopia test checkp4 remove_target relink printdependrules qmake_check qtopia_first qtopia_all qtopia_clean qtopia_distclean qtopia_install qtopia_uninstall first dist yaccclean lexclean clean distclean mocclean mocables package_qpe-dbservices compiler_moc_header_make_all compiler_moc_header_clean compiler_rcc_make_all compiler_rcc_clean compiler_image_collection_make_all compiler_image_collection_clean compiler_moc_source_make_all compiler_moc_source_clean compiler_uic_make_all compiler_uic_clean compiler_translatables_make_all dummy_translatables compiler_runcheckp4_make_all dummy_checkp4 compiler_clean install_service uninstall_service install_linstall uninstall_target install_help install_docapi_desktop install_pics qmake qmake-debug regenerate regenerate_all nodeps loud silent
