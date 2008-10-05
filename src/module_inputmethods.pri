#This file contains projects that make up the InputMethods module.

!x11 {
    PROJECTS*=\
        plugins/inputmethods/predictivekeyboard \
        plugins/inputmethods/keyboard \
        plugins/inputmethods/dockedkeyboard \
        3rdparty/plugins/inputmethods/pkim

    contains(QTOPIA_LANGUAGES,zh_CN):PROJECTS*=plugins/inputmethods/pinyin
}

PROJECTS*=\
    server/ui/components/inputmethods \
    plugins/themeitems/inputmethodsitem

PROJECTS*=\
    libraries/handwriting \
    settings/hwsettings \
    3rdparty/libraries/inputmatch \
    settings/words
