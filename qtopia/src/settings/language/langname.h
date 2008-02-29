/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// Also used by Words and QtopiaDesktop

static QString languageName(const QString& id, QFont* font)
{
    QStringList qpepaths = Global::qtopiaPaths();
    for (QStringList::Iterator qit=qpepaths.begin(); qit != qpepaths.end(); ++qit ) {
	QString tfn = *qit+"i18n/";
	QFileInfo desktopFile( tfn + "/" + id + "/.directory" );
	if( desktopFile.exists() ) {
	    // Find the name for this language...

	    Config conf( desktopFile.filePath(), Config::File );

	    QString langName;

	    //  The in-config translated name of the language.
	    if ( langName.isEmpty() )
		langName = conf.readEntryDirect( "Name["+id+"]" );

	    //  The out-of-config translated name of the language.
	    if ( langName.isEmpty() ) {
		QString engName = conf.readEntryDirect( "Name[]" );
		if ( id.left(2) == "en" )
		    langName = engName;
		else if ( !engName.isEmpty() ) {
		    QTranslator t(0);
		    if (t.load(tfn+"/"+id+"/QtopiaI18N.qm"))
			langName = t.find(engName,engName);
		}
	    }

	    //  The *untranslated* (ie. local only) name of the language.
	    if ( langName.isEmpty() )
		langName = conf.readEntryDirect( "Name" );

	    // The local-language translation of the langauge (poor)
	    // (shouldn't happen)
	    if ( langName.isEmpty() )
		langName = conf.readEntry( "Name" );

#ifndef QTOPIA_DESKTOP
	    if ( font ) {
		// OK, we have the language, now find the normal
		// font to use for that language...
		int iptsz=font->pointSize();
		QFontMetrics fm(*font);
		int i;
		for (i=0; i<(int)langName.length() && fm.inFont(langName[i]); ++i)
		    ;
		if ( i<(int)langName.length() ) {
		    QTranslator t(0);
		    if (t.load(tfn+"/"+id+"/QtopiaDefaults.qm")) {
			Config fmcfg("FontMap");
			fmcfg.setGroup("Map");
			QString bf;
			int choicegood=-1;
			for (int i=0; i<=2; ++i) {
			    bf=fmcfg.readEntry("Font"+QString::number(i)+"[]");
			    QStringList fs = QStringList::split(',',t.findMessage("FontMap",bf,0).translation());
			    if ( fs.count() == 3 ) {
				QFont tfont(fs[1],fs[2].toInt());
				QFontMetrics fm(tfont);
				int i;
				for (i=0; i<(int)langName.length() && fm.inFont(langName[i]); ++i)
				    ;
				int choicequal=abs(tfont.pointSize()-iptsz);
				if ( i==(int)langName.length() && choicequal > choicegood) {
				    choicegood = choicequal;
				    *font = tfont;
				}
			    }
			}
		    }
		}
	    }
#else
            (void)font;
#endif
	    return langName;
	}
    }
    return QString::null;
}
