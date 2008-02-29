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

class QtopiaChannel;
#ifndef SINGLE_EXEC
class QPluginManager;
class QApplicationFactoryInterface;
#endif
class QtopiaApplication;
class QEventLoop;

class QuickLauncher : public QObject
{
    Q_OBJECT
public:
    QuickLauncher();

    static void exec( int argc, char **argv );

private slots:
    void message(const QString &msg, const QByteArray & data);

private:
    void doQuickLaunch( QStringList &argList );

private:
    QtopiaChannel *qlChannel;

public:
#ifndef SINGLE_EXEC
    static QPluginManager *loader;
    static QObject *appInstance;
    static QApplicationFactoryInterface *appIface;
#endif

    static QtopiaApplication *app;
    static QPointer<QWidget> mainWindow;
    static bool validExitLoop;
    static bool needsInit;
    static QEventLoop *eventLoop;

    static char **argv0;
    static int argv_lth;
};

#endif
