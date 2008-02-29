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

#ifndef ATCOMMANDS_H
#define ATCOMMANDS_H

#include "atfrontend.h"
#include "atcallmanager.h"
#include "atcustom.h"

#include <qnetworkregistration.h>
#include <qservicenumbers.h>
#ifdef QTOPIA_CELL
#include <qcallsettings.h>
#include <QPhoneBook>
#endif

class AtCommandsPrivate;
class AtSessionManager;
class AtOptions;

class AtCommands : public QObject
{
    Q_OBJECT
public:
    AtCommands( AtFrontEnd *frontEnd, AtSessionManager *manager );
    ~AtCommands();

    AtFrontEnd *frontEnd() const;
    AtSessionManager *manager() const;
    AtOptions *options() const;

    void add( const QString& name, QObject *target, const char *slot );
    QString nextString( const QString& buf, uint& posn );
    QString quote( const QString& str );

#ifdef QTOPIA_BLUETOOTH
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
#endif

public slots:
    void send( const QString& line );
    void done( QAtResult::ResultCode result = QAtResult::OK );
    void doneWithExtendedError( QAtResult::ResultCode result );

private slots:
    void commands( const QStringList& cmds );
    void processNextCommand();
    void channelOpened( int channel, QSerialIODevice *device );
    void muxTerminated();
    void sendSignalQuality( int value );
    void signalQualityChanged( int value );
    void sendBatteryCharge( int value );
    void batteryChargeChanged( int value );

public slots:
    void ignore();
    void notAllowed();
    void ata();
    void atd( const QString& params );
    void ate( const QString& params );
    void ath();
    void ati( const QString& params );
    void ato();
    void atq( const QString& params );
    void ats3( const QString& params );
    void ats4( const QString& params );
    void ats5( const QString& params );
    void atv( const QString& params );
    void atz();
    void atampf();
    void atampw();
    void atbldn();
    void atbrsf(const QString &params);
    void atcbc( const QString& params );
    void atcbst( const QString& params );
#ifdef QTOPIA_CELL
    void atccwa( const QString& params );
#endif
    void atceer();
    void atcfun( const QString& params );
    void atcgdata( const QString& params );
    void atcgdcont( const QString& params );
    void atcgmi();
    void atcgmm();
    void atcgmr();
    void atcgsn();
    void atchld( const QString& params );
    void atchup();
    void atcimi();
    void atcind( const QString& params );
    void atckpd( const QString& params );
    void atclcc();
    void atclip( const QString& params );
    void atcmec( const QString& params );
    void atcmee( const QString& params );
    void atcmer( const QString& params );
#ifdef QTOPIA_CELL
    void atcmgf( const QString& params );
    void atcmgs( const QString& params );
    void atcmms( const QString& params );
#endif
    void atcmux( const QString& params );
    void atcnum();
    void atcops( const QString& params );
    void atcpas( const QString& params );
#ifdef QTOPIA_CELL
    void atcpbf( const QString& params );
    void atcpbr( const QString& params );
    void atcpbs( const QString& params );
    void atcpbw( const QString& params );
#endif
    void atcpin( const QString& params );
#ifdef QTOPIA_CELL
    void atcpms( const QString& params );
#endif
    void atcrc( const QString& params );
    void atcreg( const QString& params );
    void atcres();
    void atcsas();
#ifdef QTOPIA_CELL
    void atcsca( const QString& params );
#endif
    void atcscs( const QString& params );
#ifdef QTOPIA_CELL
    void atcsdh( const QString& params );
    void atcsmp( const QString& params );
    void atcsms( const QString& params );
#endif
    void atcsq( const QString& params );
    void atcsta( const QString& params );
    void atgcap();
    void atgmi();
    void atgmm();
    void atgmr();
    void atgsn();
#ifndef QTOPIA_AT_STRICT
    void atqbc( const QString& params );
    void atqcam( const QString& params );
    void atqsq( const QString& params );
#endif
    void atvgm(const QString &params);
    void atvgs(const QString &params);
    void atvtd( const QString& params );
    void atvts( const QString& params );
    void atnrec(const QString &params);
    void atbvra(const QString &params);
    void atbinp(const QString &params);
    void atbtrh(const QString &params);

private:
    AtCommandsPrivate *d;

    int soption( const QString& params, int prev, int min, int max );

    enum Mode
    {
        CommandOnly,            // AT+CFOO only
        Get,                    // AT+CFOO?
        Set,                    // AT+CFOO=value
        Support,                // AT+CFOO=?
        Error                   // Syntax error
    };

    Mode mode( const QString& params ) const;
    void flagCommand( const QString& prefix, bool& flag,
                      const QString& params, const QString& extra=QString() );
    void needIndicators();
    void queryNumber( QServiceNumbers::NumberId id );
    void setNumber( QServiceNumbers::NumberId id, const QString& value );

#ifdef QTOPIA_CELL
    void writeMemoryPhoneBookEntry( bool isDeletion, uint index, const QString& number = QString(), const QString& text = QString() );
#endif

private slots:
    void stateChanged( int callID, AtCallManager::CallState state,
                       const QString& number, const QString& type );
    void deferredResult( AtCommands *handler, QAtResult::ResultCode result );
    void ring( const QString& number, const QString& type );
    void callWaiting( const QString& number, const QString& type );
    void noCarrier();
    void indicatorChanged( int ind, int value );
#ifdef QTOPIA_CELL
    void callWaitingState( QTelephony::CallClass cls );
    void setCallWaitingResult( QTelephony::Result result );
#endif
    void registrationStateChanged();
    void availableOperators
            ( const QList<QNetworkRegistration::AvailableOperator>& opers );
    void setCurrentOperatorResult( QTelephony::Result result );
    void serviceNumber( QServiceNumbers::NumberId id, const QString& number );
    void setServiceNumberResult( QServiceNumbers::NumberId id, QTelephony::Result result );
    void sendNextKey();
#ifdef QTOPIA_CELL
    void extraLine( const QString& line, bool cancel );
    void smsFinished( const QString& id, QTelephony::Result result );
    void setLevelResult( QTelephony::Result result );
    void phoneBookLimits( const QString& store, const QPhoneBookLimits& limits );
    void phoneBookEntries( const QString& store, const QList<QPhoneBookEntry>& entries );
    void initializeMemoryPhoneBook();
#endif
};

#endif // ATCOMMANDS_H
