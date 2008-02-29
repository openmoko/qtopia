/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

// Also used by Words, Date & Time
#include <stdlib.h>
#include <QTranslator>

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

            if ( font ) {
                // OK, we have the language, now find the normal
                // font to use for that language...
                //don't use QFontMetric::inFont() as it increases startup time by 10%
                QTranslator t(0);
                QString filename = tfn + id + "/QtopiaDefaults.qm";
                QSettings qpeconfig(QSettings::SystemScope, QLatin1String("Trolltech"), QLatin1String("qpe"));
                qpeconfig.beginGroup("Font");
                if (t.load(filename)) {
                    QString trFamily = t.translate("QPE", qpeconfig.value(QLatin1String("FontFamily[]")).toString().toAscii().constData());
                    QString trSize = t.translate("QPE", qpeconfig.value(QLatin1String("FontSize[]")).toString().toAscii().constData());

                    QFont f(trFamily);
                    if (!trSize.isEmpty())
                        f.setPointSizeF(trSize.toDouble());
                    *font = f;
                } else {
                    //if we cannot find qm file it must be en_US or
                    //none specified for id -> keep dejavu
                    double size = qpeconfig.value(QLatin1String("FontSize[]")).toDouble();

                    QFont f(font->family());
                    f.setPointSizeF(size);
                    *font = f;
                    qLog( I18n ) << "Using default font" << font->family() << "for" << id;
                }
            }
            return langName;
        }
    }
    return QString();
}
