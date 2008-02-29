/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef QIR_H
#define QIR_H

#include <qobject.h>
#include <qtimer.h>

class QCopChannel;
class QIrServer;
class ReceiveDialog;
class SendWindow;

class IrPowerSave;

class QIr : public QObject
{
    Q_OBJECT
public:
    QIr( QObject *parent = 0, const char *name = 0 );
    ~QIr();

public slots:
    void obexMessage(const QCString&, const QByteArray&);
    void traySocket( const QCString&, const QByteArray &);

    void receiveInit();
    void receiveError();

    void beamNext();
    void beamDone();
    void beamError();
    
private slots:
    void receiving( int len, const QString &filename, const QString &mimetype );
    void receiveDone();

private:
    QIrServer *obexServer;
    QCopChannel *obexChannel, *trayChannel;
    
    ReceiveDialog *receiveWindow;
    SendWindow *sendWindow;
    IrPowerSave *ips;
};

class IrPowerSave : public QObject
{
    Q_OBJECT
public:
    IrPowerSave( QObject *parent = 0, const char *name = 0);
    ~IrPowerSave();
    
    enum State { Off, On, OnMinutes, On1Item };

    void initBeam();
    void beamingDone(bool received = FALSE);

public slots:
    void obexMessage(const QCString&, const QByteArray&);
    
private slots:
    void timeout();

private:
    void applyReceiveState(State s);
    void service(const QString &);

private:
    State state;
    bool running;
    bool inUse;
    int time;
    QTimer *timer;
};

#endif
