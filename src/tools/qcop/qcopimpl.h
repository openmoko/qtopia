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

#ifndef QCOPIMPL_H
#define QCOPIMPL_H

#ifndef QT_NO_COP
#include <private/qcopenvelope_p.h>
#endif

#include <qstringlist.h>
#include <qdatastream.h>
#include <qtimer.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifndef Q_OS_WIN32
#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#endif

// Base handler class that supports chaining.
class QCopHandler : public QObject
{
    Q_OBJECT
public:
    QCopHandler( QObject *parent = 0 );
    ~QCopHandler();

public slots:
    virtual void start( bool ok );

signals:
    void done( bool );
};

// Handler at the end of the chain which exits with 0 or 1 status code.
class QCopEndHandler : public QCopHandler
{
    Q_OBJECT
public:
    QCopEndHandler( QObject *parent = 0 );
    ~QCopEndHandler();

public slots:
    void start( bool ok );

private slots:
    void succeed();
    void fail();
};

// Handler that sends a specific QCop message.
class QCopSender : public QCopHandler
{
    Q_OBJECT
public:
    QCopSender( QCopEnvelope *env, QObject *parent = 0 );
    ~QCopSender();

public slots:
    void start( bool ok );

private slots:
    void finish();

private:
    QCopEnvelope *env;
};

// Handler that watches for all incoming messages on a channel.
class QCopWatcher : public QCopHandler
{
    Q_OBJECT
public:
    QCopWatcher( const QString& channel, int timeout = -1,
                 bool hexflag = true, QObject *parent = 0 );
    ~QCopWatcher();

public slots:
    void start( bool ok );
    void addExtraWatch( const QString& channel );

private slots:
    void received( const QString& msg, const QByteArray& data );
    void gotTimeout();

private:
    QStringList channels;
    int timeout;
    bool hexflag;

    void printMessage( const QString& msg, const QByteArray& data );
};

// Handler that waits for a specific message on a channel.
class QCopWaiter : public QCopHandler
{
    Q_OBJECT
public:
    QCopWaiter( const QString& channel, const QString& msg,
                int timeout = -1, bool hexflag = true, QObject *parent = 0 );
    ~QCopWaiter();

    void setExitOnTimeout() { exitOnTimeout = true; }

public slots:
    void start( bool ok );

signals:
    void done( bool ok );

private slots:
    void received( const QString& msg, const QByteArray& data );
    void gotTimeout();

private:
    QString channel;
    QString msg;
    int timeout;
    bool hexflag;
    bool finished;
    bool exitOnTimeout;
};

// Handler that issues a query and waits for the responses.
class QCopQuery : public QCopHandler
{
    Q_OBJECT
public:
    QCopQuery( const QString& channel, const QString& prefix,
               int timeout = -1, QObject *parent = 0 );
    ~QCopQuery();

public slots:
    void start( bool ok );

private slots:
    void received( const QString& msg, const QByteArray& data );
    void gotTimeout();

private:
    QString channel;
    QString prefix;
    int timeout;
    QStringList responses;
    bool finished;
};

#endif
