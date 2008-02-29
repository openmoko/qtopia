/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
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
    void received( const QString&, const QString &);
    void receiveError();

    void beamNext();
    void beamDone();
    void beamError();
    
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
