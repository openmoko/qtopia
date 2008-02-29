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

#ifndef QTOPIANAMESPACE_H
#define QTOPIANAMESPACE_H

#include <qtopiaglobal.h>

#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QDir>
#include <QRegExp>
#include <QList>
#include <QTranslator>
#include <QSettings>

class QContentSet;
class QDawg;

namespace Qtopia
{
    /*

    Global functions

    */

    QTOPIABASE_EXPORT QStringList installPaths();
    QTOPIABASE_EXPORT QString updateDir();
    QTOPIABASE_EXPORT QString qtopiaDir();
    QTOPIABASE_EXPORT QString packagePath();
    QTOPIABASE_EXPORT QString documentDir();
    QTOPIABASE_EXPORT QString defaultButtonsFile();
    QTOPIABASE_EXPORT QString homePath();
    QTOPIABASE_EXPORT QStringList helpPaths();

    QTOPIABASE_EXPORT bool truncateFile(QFile &f, int size);
    QTOPIABASE_EXPORT QString tempDir( );
    QTOPIABASE_EXPORT QString tempName(const QString &filename);

    QTOPIABASE_EXPORT QString sandboxDir();
    QTOPIABASE_EXPORT QString applicationFileName(const QString& appname, const QString& filename);

    QTOPIABASE_EXPORT bool isDocumentFileName(const QString& file);

    enum Lockflags {LockShare = 1, LockWrite = 2, LockBlock = 4};
    QTOPIABASE_EXPORT bool lockFile(QFile &f, int flags = -1);
    QTOPIABASE_EXPORT bool unlockFile(QFile &f);
    QTOPIABASE_EXPORT bool isFileLocked(QFile &f, int flags = -1);

    QTOPIABASE_EXPORT bool mousePreferred();
    QTOPIABASE_EXPORT bool hasKey(int key);

#ifndef QTOPIA_CONTENT_INSTALLER
    QTOPIABASE_EXPORT void execute(const QString &exec, const QString &document=QString());
#endif

    /*

    User interface segment

    */

    QTOPIABASE_EXPORT bool confirmDelete(QWidget *parent, const QString &caption, const QString &object);
    QTOPIABASE_EXPORT void actionConfirmation(const QPixmap &pix, const QString &text);
    QTOPIABASE_EXPORT void soundAlarm();
    QTOPIABASE_EXPORT void statusMessage(const QString&);

    /*

    Localization functions

    */

    QTOPIABASE_EXPORT QString translate(const QString& key, const QString& c, const QString& str);
    QTOPIABASE_EXPORT QStringList languageList();

    QTOPIABASE_EXPORT bool weekStartsOnMonday();
    QTOPIABASE_EXPORT void setWeekStartsOnMonday(bool );

    QTOPIABASE_EXPORT QVariant findDisplayFont(const QString &s);

    /*

    Real-time clock functions

    */
    QTOPIABASE_EXPORT void addAlarm ( QDateTime when, const QString& channel, const QString& msg, int data=0);
    QTOPIABASE_EXPORT void deleteAlarm (QDateTime when, const QString& channel, const QString& msg, int data=0);
    QTOPIABASE_EXPORT void writeHWClock();

    /*

    String manipulation utility functions

    */

    QTOPIABASE_EXPORT QString simplifyMultiLineSpace( const QString &multiLine );
    QTOPIABASE_EXPORT QString dehyphenate(const QString&);

    QTOPIABASE_EXPORT QString shellQuote(const QString& s);
    QTOPIABASE_EXPORT QString stringQuote(const QString& s);

    // System independant sleep
    QTOPIABASE_EXPORT void sleep(unsigned long secs);
    QTOPIABASE_EXPORT void msleep(unsigned long msecs);
    QTOPIABASE_EXPORT void usleep(unsigned long usecs);

    QTOPIABASE_EXPORT QString version();
    QTOPIABASE_EXPORT QString architecture();
    QTOPIABASE_EXPORT QString deviceId();
    QTOPIABASE_EXPORT QString ownerName();

    /*

    Dictionary  functions

    */

    // Dictionaries
    QTOPIABASE_EXPORT const QDawg& fixedDawg();
    QTOPIABASE_EXPORT const QDawg& addedDawg();
    QTOPIABASE_EXPORT const QDawg& dawg(const QString& name, const QString& language = QString());

    QTOPIABASE_EXPORT void addWords(const QStringList& word);
    QTOPIABASE_EXPORT void addWords(const QString& dictname, const QStringList& word);
    QTOPIABASE_EXPORT void removeWords(const QStringList& word);
    QTOPIABASE_EXPORT void removeWords(const QString& dictname, const QStringList& word);
    QTOPIABASE_EXPORT void qtopiaReloadWords( const QString& dictname );

    /*

    Qtopia enums

    */
    enum ItemDataRole {
        AdditionalDecorationRole = 1000,
        UserRole = 2000
    };

}
#endif
