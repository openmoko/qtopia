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
#ifndef GLOBAL_H
#define GLOBAL_H

#include <qtopia/qpeglobal.h>
#include <qtopia/quuid.h>
#include <qstringlist.h>
#include <qguardedptr.h>
#include <qwidget.h>

class QDawg;
class QLabel;
class AppLnk;
class DocLnkSet;
class QFile;

#if defined(__GNUC__) && !defined(NO_DEBUG)
// C99 defines __func__ instead of __FUNCTION__, possibly supported on windows
# include <time.h>
# include <sys/times.h>
# define QTOPIA_HAVE_PROFILE
# define QTOPIA_PROFILE(s)    extern struct tms qtopia_prof_times; qDebug("%fms %s (%s), line: %i file: %s", times(&qtopia_prof_times)*1000.0/CLOCKS_PER_SEC, __FUNCTION__, s, __LINE__, __FILE__ )
#else
# define QTOPIA_PROFILE(s)
#endif

class QTOPIA_EXPORT Global
{
public:
    static QUuid generateUuid(); // libqtopia

    static QStringList qtopiaPaths(); // libqtopia2
    static QString defaultButtonsFile(); // libqtopia2
    static QString homeDirPath(); // libqtopia
    static bool renameFile( QString from, QString to ); // libqtopia
    static QString tempName(const QString &filename); // libqtopia
    static QString journalFileName(const QString &filename); // libqtopia

    static QString applicationFileName(const QString& appname, const QString& filename);
#ifdef Q_WS_QWS
    static void findDocuments(DocLnkSet* folder, const QString &mimefilter=QString::null);
#endif
    static bool isDocumentFileName(const QString& file); // libqtopia
    static bool isAppLnkFileName(const QString& file); // libqtopia

#ifdef QTOPIA_INTERNAL_FILEOPERATIONS
    static bool truncateFile(QFile &f, int size);
    static QString tempDir( );
#endif

#ifdef QTOPIA_INTERNAL_LANGLIST
    static QStringList languageList();
    static QStringList helpPath();
#endif

    static void statusMessage(const QString&);

    static QString shellQuote(const QString& s);
    static QString stringQuote(const QString& s);

    static QByteArray encodeBase64(const QByteArray& origData); // libqtopia
    static QByteArray decodeBase64(const QByteArray& encoded); // libqtopia
    static bool weekStartsOnMonday(); // libqtopia
    static void setWeekStartsMonday(bool ); // libqtopia

    static bool mousePreferred(); // libqtopia2
    static bool hasKey(int key); // libqtopia2

    // System independant sleep
    static void sleep( unsigned long secs ); // libqtopia2
    static void msleep( unsigned long msecs ); // libqtopia2
    static void usleep( unsigned long usecs ); // libqtopia2

    // libqtopiamail relies on these so they need to be available to Qtopia Desktop
#ifdef QTOPIA_DESKTOP
    enum Lockflags {LockShare = 1, LockWrite = 2, LockBlock = 4};
    static bool lockFile(QFile &f, int flags = -1);
    static bool unlockFile(QFile &f);
    static bool isFileLocked(QFile &f, int flags = -1);
#endif

#ifdef Q_WS_QWS

    Global();

    // Dictionaries
    static const QDawg& fixedDawg();
    static const QDawg& addedDawg();
    static const QDawg& dawg(const QString& name);

    static void addWords(const QStringList& word);
    static void addWords(const QString& dictname, const QStringList& word);
#if defined(QTOPIA_PHONE) || defined(QTOPIA_INTERNAL_DICTOPERATIONS)
    static void removeWords(const QStringList& word);
    static void removeWords(const QString& dictname, const QStringList& word);
#endif

    static void createDocDir();

    static QString version(); // libqtopia
    static QString architecture(); // libqtopia
    static QString deviceId(); // libqtopia
    static QString ownerName(); // libqtopia

    struct Command {
	const char *file;
	QWidget *(*func)( bool );
	bool maximized;
	bool documentary;
    };
    static void setBuiltinCommands( Command* );
    static Command* builtinCommands();
    static QGuardedPtr<QWidget>* builtinRunning();

    static void execute( const QString &exec, const QString &document=QString::null );
    static void setDocument( QWidget* receiver, const QString& document );
    static bool terminateBuiltin( const QString& );
    static void terminate( const AppLnk* );

    static bool isBuiltinCommand( const QString &name );

    // system messaging
    static void applyStyle();
    static QWidget *shutdown( bool = FALSE );
    static QWidget *restart( bool = FALSE );
    static void hideInputMethod();
    static void showInputMethod();

    static void writeHWClock();

#ifdef QTOPIA_INTERNAL_FILEOPERATIONS
    enum Lockflags {LockShare = 1, LockWrite = 2, LockBlock = 4};
    static bool lockFile(QFile &f, int flags = -1);
    static bool unlockFile(QFile &f);
    static bool isFileLocked(QFile &f, int flags = -1);
#endif

private:
    static void invoke( const QString &exec);
    static Command* builtin;
    static QGuardedPtr<QWidget> *running;
    static void removeRunningArray();
#endif
};

#ifdef QTOPIA_INTERNAL_FILEOPERATIONS
QString qtopia_tempName( const QString &fname );
bool qtopia_renameFile( QString from, QString to );
#endif

#endif
