/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef ATGSMNONCELLCOMMANDS_H
#define ATGSMNONCELLCOMMANDS_H


#include "atsessionmanager.h"
#include "atoptions.h"
#include "atindicators.h"
#include "atcustom.h"

#include <QObject>
#include <qtopiaservices.h>
#include <QPair>
#include <QNetworkRegistration>
#include <QServiceNumbers>
#include <QDateTime>

class AtCommands;
class QPhoneProfileManager;
class QGsm0710MultiplexerServer;

class AtGsmNonCellCommands : public QObject
{
    Q_OBJECT

public:
    AtGsmNonCellCommands( AtCommands * parent );
    ~AtGsmNonCellCommands();

    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void queryNumber( QServiceNumbers::NumberId id );
    void setNumber( QServiceNumbers::NumberId id, const QString& value );
    void flagCommand( const QString& prefix, bool& flag,
                      const QString& params, const QString& extra=QString() );
    QNetworkRegistration* networkRegistration();

    bool sendRelease;

public slots:
    void atcalm( const QString& params );
    void atcbc( const QString& params );
    void atcbst( const QString& params );
    void atcclk( const QString& params );
    void atceer( const QString& params );
    void atcgdata( const QString& params );
    void atcgdcont( const QString& params );
    void atcgmi( const QString& params );
    void atcgmm( const QString& params );
    void atcgmr( const QString& params );
    void atcgsn( const QString& params );
    void atchld( const QString& params );
    void atchup( const QString& params );
    void atcind( const QString& params );
    void atckpd( const QString& params );
    void atclae( const QString& params );
    void atclan( const QString& params);
    void atclcc( const QString& params );
    void atclip( const QString& params );
    void atclvl( const QString& params );
    void atcmec( const QString& params );
    void atcmee( const QString& params );
    void atcmer( const QString& params );
    void atcmod( const QString& params );
    void atcmut( const QString& params );
    void atcmux( const QString& params );
    void atcnum( const QString& params );
    void atcops( const QString& params );
    void atcpas( const QString& params );
    void atcpin( const QString& params );
    void atcr(const QString& params );
    void atcrc( const QString& params );
    void atcreg( const QString& params );
    void atcrmc( const QString& params );
    void atcrmp( const QString& params );
    void atcscs( const QString& params );
    void atcsdf( const QString& params );
    void atcsgt( const QString& params );
    void atcsil( const QString& params );
    void atcsns( const QString& params );
    void atcsq( const QString& params );
    void atcssn( const QString& params );
    void atcsta( const QString& params );
    void atcstf( const QString& params );
    void atcsvm( const QString& params );
    void atctfr( const QString& params );
    void atcvhu( const QString& params );
    void atcvib( const QString& params );
    void atqbc( const QString& params );
    void atqcam( const QString& params );
    void atqsq( const QString& params );
    void atvtd( const QString& params );
    void atvts( const QString& params );

private slots:
    void channelOpened( int channel, QSerialIODevice *device );
    void muxTerminated();

    void indicatorChanged( int ind, int value );
    void batteryChargeChanged( int value );

    void alarmHandler( const QString& msg, int data );

    void registrationStateChanged();
    void availableOperators
            ( const QList<QNetworkRegistration::AvailableOperator>& opers );
    void setCurrentOperatorResult( QTelephony::Result result );
    void serviceNumber( QServiceNumbers::NumberId id, const QString& number );
    void setServiceNumberResult( QServiceNumbers::NumberId id, QTelephony::Result result );
    void sendNextKey();

    void sendSignalQuality( int value );
    void signalQualityChanged( int value );

private:
    int getSilentPhoneProfileId();
    void needIndicators();
    void sendBatteryCharge( int value );

    AtCommands *atc;

    QServiceNumbers *serviceNumbers;
    QServiceNumbers::NumberId pendingQuery;
    QServiceNumbers::NumberId pendingSet;

    QPhoneProfileManager *gqppm;

    AtPhoneIndicators *indicators;
    QNetworkRegistration *netReg;
    bool requestingAvailableOperators;
    bool settingCurrentOperator;
    int activePhoneProfileId;
    int keyPressTime;
    int keyPauseTime;
    QList< QPair<int, int> > keys;

    QGsm0710MultiplexerServer *multiplexer;
    QSerialIODevice *underlying;

    struct alarm_info {
        QString identifier;
        int nid;
        QString message;
        QDateTime time;
        int type;
        int recurrant;
        bool silent;
    };
    QList<alarm_info> alarmList;
};

#endif // ATGSMNONCELLCOMMANDS_H


