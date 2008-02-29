/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

class QTOPIA_EXPORT Global
{
public:
    static QUuid generateUuid();

    static QString homeDirPath();
    static QString journalFileName(const QString &filename);
    static QString applicationFileName(const QString& appname, const QString& filename);
    static void findDocuments(DocLnkSet* folder, const QString &mimefilter=QString::null);
    static bool isDocumentFileName(const QString& file);
    static bool isAppLnkFileName(const QString& file);

#ifdef QTOPIA_INTERNAL_LANGLIST
    static QStringList languageList();
    static QStringList helpPath();
#endif

    static void statusMessage(const QString&);

#ifdef Q_WS_QWS

    Global();

    // Dictionaries
    static const QDawg& fixedDawg();
    static const QDawg& addedDawg();
    static const QDawg& dawg(const QString& name);

    static void addWords(const QStringList& word);
    static void addWords(const QString& dictname, const QStringList& word);
    // static void removeWords(const QStringList& word); -- if someone wants it

    static void createDocDir();

    static QString deviceId();
    static QString ownerName();

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

    static QString shellQuote(const QString& s);
    static QString stringQuote(const QString& s);

#ifdef QTOPIA_FILEOPERATIONS
    static bool truncateFile(QFile &f, int size);

    enum Lockflags {LockShare = 1, LockWrite = 2, LockBlock = 4};
    static bool lockFile(QFile &f, int flags = -1);
    static bool unlockFile(QFile &f);
    static bool isFileLocked(QFile &f, int flags = -1);
#endif

private:
    static void invoke( const QString &exec);
    static Command* builtin;
    static QGuardedPtr<QWidget> *running;
#endif
};


#endif
