/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef INETDADAPTOR_H
#define INETDADAPTOR_H

#include <QList>
#include <QObject>
#include <QMutex>
#include <QProcess>

class QTcpServer;
class HttpWorker;

class InetdAdaptor : public QObject
{
    Q_OBJECT
public:
    InetdAdaptor( QObject *parent = 0 );
    ~InetdAdaptor();
    static InetdAdaptor *getInstance();
    bool isRunning() const;
    void start();
    void stop();
signals:
    void progressValue( int );
    void adaptorMessage( const QString & );
    void startedRunning();
    void stoppedRunning();
private slots:
    void newConnection();
    void httpdError( QProcess::ProcessError );
    void socketReadyRead();
    void httpdReadyRead();
    void disconnected();
private:
    void runMicroHttpd( HttpWorker *worker );
    QTcpServer *mTcpServer;
    QList<HttpWorker*> mWorkers;
    HttpWorker *mShowingProgress;
    QMutex mWorkersLock;
    QMutex mServerLock;
    QMutex mShowingProgressLock;
};

#endif
