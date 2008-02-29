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

#ifndef IAXTELEPHONYSERVICE_H
#define IAXTELEPHONYSERVICE_H

#include <qobject.h>
#include <qtopiaabstractservice.h>
#include <qtelephonyservice.h>

class QAudioInput;
class QAudioOutput;
class QTimer;
class IaxNetworkRegistration;

class IaxTelephonyService : public QTelephonyService
{
    Q_OBJECT
public:
    IaxTelephonyService( const QString& service, QObject *parent = 0 );
    ~IaxTelephonyService();

    void initialize();

    // Callbacks that interface to the iaxclient library.
    bool startAudio();
    void stopAudio();
    int writeAudio( const void *samples, int numSamples );
    int readAudio( void *samples, int numSamples );
    bool processEvent( struct iaxc_event_struct *e );

public slots:
    void serviceIaxClient();
    void updateRegistrationConfig();
    void updateCallerIdConfig();

private slots:
    void retryAudioOpen();

private:
    bool initialized;
    QAudioInput *audioInput;
    QAudioOutput *audioOutput;
    QTimer *timer;
    int retryCount;
    int interval;
    QTimer *iaxTimer;

    IaxNetworkRegistration *netReg() const;
};

class IaxTelephonyServiceQCop : public QtopiaAbstractService
{
    Q_OBJECT

public:
    IaxTelephonyServiceQCop( QObject *parent = 0 );
    ~IaxTelephonyServiceQCop();

public slots:
    void start();
    void stop();

private:
    IaxTelephonyService *service;
};

#endif /* IAXTELEPHONYSERVICE_H */
