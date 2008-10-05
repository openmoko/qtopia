TEMPLATE=subdirs
SUBDIRS=src etc/themes examples

# qbuild cleanimage
cleanimage [
    TYPE=RULE
    commands=\
        "#(eh)echo $$shellQuote(Removing $$QTOPIA_IMAGE)"\
        "#(e)rm -rf $$shellQuote($$QTOPIA_IMAGE)"
]

# qbuild clearDBlocks
clearDBlocks [
    TYPE=RULE
    commands=\
        "#(e)$$path(QtopiaSdk:/bin/content_installer,generated) -clearlocks $$QTOPIA_IMAGE/qtopia_db.sqlite"\
        "#(e)for qtopia in /tmp/qtopia-*; do \
                 if [ -f $$shellQuote($qtopia/$$QTOPIA_IMAGE/qtopia_db.sqlite) ]; then \
                     $$path(QtopiaSdk:/bin/content_installer,generated) -clearlocks $$shellQuote($qtopia/$$QTOPIA_IMAGE/qtopia_db.sqlite); \
                 fi; \
             done"
    prerequisiteActions=/src/tools/content_installer/target
]

# Before qbuild image, remove the image and clear the DB locks
PREIMAGE_RULES=cleanimage clearDBlocks sub_prep_db

# QBuild append_image skips the removing part. Not so useful now but will be once the install
# rules set inputFiles and outputFiles correctly (ie. only update what isn't already up-to-date).
append_image [
    TYPE=RULE
    prerequisiteActions=clearDBlocks sub_prep_db sub_image makeimagedone
    serial=true
]

# print a message when qbuild is done
makedone [
    TYPE=RULE
    commands=\
        "#(eh)echo $$shellQuote()"\
        "#(eh)echo $$shellQuote(Qtopia has been built.)"\
        "#(eh)echo $$shellQuote()"\
        "#(eh)echo $$shellQuote(You must now install Qtopia by running 'bin/qbuild image'.)"\
        "#(eh)echo $$shellQuote(This will put the files required to run Qtopia into the image:)"\
        "#(eh)echo $$shellQuote($$QTOPIA_IMAGE)"\
        "#(eh)echo $$shellQuote()"
]
POSTRUN_RULES=makedone

# print a message when qbuild image is done
makeimagedone.TYPE=RULE
makeimagedone.commands=\
    "#(eh)echo $$shellQuote()"\
    "#(eh)echo $$shellQuote(Qtopia has been installed.)"\
    "#(eh)echo $$shellQuote()"\
    "#(eh)echo $$shellQuote(The files required to run Qtopia are in the image:)"\
    "#(eh)echo $$shellQuote($$QTOPIA_IMAGE)"\
    "#(eh)echo $$shellQuote()"
equals(QTOPIA_IMAGE,$$QTOPIA_PREFIX):makeimagedone.commands+=\
    "#(eh)echo $$shellQuote(Please note that Qtopia cannot be moved. It must be run from the image.)"
else:makeimagedone.commands+=\
    "#(eh)echo $$shellQuote(Please note that Qtopia cannot be run from the image.)"\
    "#(eh)echo $$shellQuote(You must move Qtopia to the prefix first. The prefix is:)"\
    "#(eh)echo $$shellQuote($$QTOPIA_PREFIX)"
!enable_rpath:makeimagedone.commands+=\
    "#(eh)echo $$shellQuote()"\
    "#(eh)echo $$shellQuote(Note that you will need to set LD_LIBRARY_PATH=$$QTOPIA_PREFIX/lib to run Qtopia.)"
makeimagedone.commands+=\
    "#(eh)echo $$shellQuote()"
POSTIMAGE_RULES=makeimagedone

# Setup some default targets
installs_getImage()

!equals(SDKROOT,$$path(/,generated)) {
    # make the SDK stand-alone
    <script>
    var script = project.property("SDK_SCRIPT");
    var file = qbuild.invoke("path", "/src/build/bin/sdkcache", "project");
    script.setValue(file);
    if ( script.strValue() == "" ) {
        project.warning("Unable to locate sdkcache script.");
        return;
    }
    </script>
    sdk.TYPE=RULE
    sdk.prerequisiteActions=default
    sdk.commands=\
        "#(eh)echo $$shellQuote(Creating a stand-alone SDK)"\
        "#(E)$$SDK_SCRIPT $$SDKROOT"
}

