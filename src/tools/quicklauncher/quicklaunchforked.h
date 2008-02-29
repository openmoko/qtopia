/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#ifndef QUICKLAUNCH_H
#define QUICKLAUNCH_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QPointer>
#include <QWidget>
#include <QMap>
#include <QList>
#include <QCopChannel>

class QtopiaChannel;
#ifndef SINGLE_EXEC
class QPluginManager;
class QApplicationFactoryInterface;
#endif
class QtopiaApplication;
class QEventLoop;
class QSocketNotifier;
class QtopiaIpcAdaptor;

// ============================================================================
//
//  QuickLauncher
//
// ============================================================================

class QuickLauncher : public QObject
{
    Q_OBJECT
public:
    QuickLauncher();
    ~QuickLauncher();

    static void exec( int argc, char **argv );

public slots:
    void childDied();
    void fillPool();
    void listenToChannel();
    void disconnectChannel();

    void execute( const QStringList& arguments );
    void execute( const QString& argument );
    void run( const QStringList& arguments );
    void quit();

private:
    void doQuickLaunch( const QStringList& argList );
    void runApplication( const QStringList& argList );
    void reinit();

private:
    QtopiaIpcAdaptor* channel;
    QMap<int, QString> apps;
    QList<int> pool;

public:
#ifndef SINGLE_EXEC
    static QPluginManager *loader;
    static QObject *appInstance;
    static QApplicationFactoryInterface *appIface;
#endif

    static QtopiaApplication *app;
    static QPointer<QWidget> mainWindow;
    static bool validExitLoop;
    static QEventLoop *eventLoop;
    static QSocketNotifier* notifier;
    static int ql_deadChild_pipe[2];

    static char **argv0;
    static int argv_lth;
};

#endif
