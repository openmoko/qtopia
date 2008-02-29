/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Also used by Words and QtopiaDesktop
#include <stdlib.h>
#include <QTranslator>
#include <QVariant>

#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qtranslatablesettings.h>

static QString languageName(const QString& id, QFont* font, bool* rightToLeft)
{
    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::Iterator qit=qpepaths.begin(); qit != qpepaths.end(); ++qit ) {
        QString tfn = *qit+"i18n/";
        QFileInfo desktopFile( tfn + id + "/.directory" );
        if( desktopFile.exists() ) {
            // Find the name for this language...

            QTranslatableSettings conf(desktopFile.filePath(), QSettings::IniFormat);
            conf.beginGroup(QLatin1String("Desktop Entry"));
            QString langName;
            //  The out-of-config translated name of the language.
            if ( langName.isEmpty() ) {
                QString engName = conf.value( QLatin1String("Name[]") ).toString();
                if ( id.left(2) == "en" )
                    langName = engName;
                else if ( !engName.isEmpty() ) {
                    QTranslator t(0);
                    if (t.load(tfn+QChar('/')+id+QLatin1String("/QtopiaI18N.qm")))
                        langName = t.translate(engName.toAscii().constData(),engName.toAscii().constData());
                    if ( rightToLeft && t.load(tfn+QLatin1Char('/')+id+QLatin1String("/qt.qm")))
                        *rightToLeft = (t.translate("QApplication","QT_LAYOUT_DIRECTION") == "RTL" );
                }
            }

            // The local-language translation of the language (poor)
            // (shouldn't happen)
            if ( langName.isEmpty() )
                langName = conf.value( QLatin1String("Name") ).toString();

#ifndef QTOPIA_DESKTOP
            if ( font ) {
                // OK, we have the language, now find the normal
                // font to use for that language...
                //don't use QFontMetric::inFont() as it increases startup time by 10%
                int iptsz=font->pointSize();
                QTranslator t(0);
                QString filename = tfn+id+"/QtopiaDefaults.qm";
                if (t.load(filename)) {
                    QSettings fmcfg(QSettings::SystemScope, QLatin1String("Trolltech"), QLatin1String("FontMap"));
                    fmcfg.beginGroup(QLatin1String("Map"));
                    QString fonts[3];
                    int size[3];
                    QString bf;
                    QStringList fs;
                    for ( int i=0; i<3; ++i ) {
                        bf = fmcfg.value( "Font"+QString::number(i)+"[]" ).toString();
                        QString tr = t.translate("FontMap",bf.toLatin1().constData());
                        if ( !tr.isEmpty() )
                            bf = tr;
                        fs = bf.split(",");
                        fonts[i] = fs[1];
                        size[i] = fs[2].toInt();
                    }
                    int i;
                    for ( i=0; i<3 && size[i] < iptsz; ++i ) ;
                    if ( i == 3 )
                        *font = QFont( fonts[2], size[2] );
                    else
                        *font = QFont( fonts[i], size[i] );
                } else {
                    //if we cannot find qm file it must be en_US or
                    //none specified for id -> keep dejavu
                    qLog( I18n ) << "Using default font" << font->family() << "for" << id;
                }
            }
#else
            (void)font;
#endif
            return langName;
        }
    }
    return QString();
}
