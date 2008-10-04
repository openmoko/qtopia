/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

Language = {
    ensureInputLanguages: function(languages)
    {
        if (languages.count == 0) return;
        var list = getDirectoryEntries( "$QPEDIR/i18n", QDir.NoDotAndDotDot | QDir.Dirs );
        for (var i = 0; i < languages.count; ++i) {
            if ( !list.contains(languages[i]) ) {
                fail("Necessary languages not installed");
                return;
            }
        }
    },

    selectLanguage: function( language )
    {
    var l;
    switch (language) {
        case "Deutsch":
            l = "de";
            break;
        case "English (U.S.)":
            l = "en_US";
            break;
        case "RTL English":
            l = "en_SU";
            break;
    }
    if( getSetting("$HOME/Settings/Trolltech/locale.conf", "Language", "Language") != l ){
        setSetting( "$HOME/Settings/Trolltech/locale.conf", "Language", "Language", l );
        restartQtopia();
    }
    // TODO: Do through UI - restarting currently fails
/*
    startApplication( "language" );
    var availablelanguages = getList();
    expectMessageBox( "Language Change", "This will cause Qt Extended to restart, closing all applications.", "Yes" ){
    select( language );
    }
    waitForQtopiaStart();
*/

    },

    useForInput: function( languages )
    {
        if( getSetting("$HOME/Settings/Trolltech/locale.conf", "Language", "InputLanguages") != languages ){
            setSetting( "$HOME/Settings/Trolltech/locale.conf", "Language", "InputLanguages", languages );
            //restartQtopia();
        }

        // TODO: Do through UI
/*
    startApplication( language );
    UserInterface.highlight(language);
    select( "Use for input", optionsMenu() );
    select( "Back", softMenu() );
*/
    }
}
