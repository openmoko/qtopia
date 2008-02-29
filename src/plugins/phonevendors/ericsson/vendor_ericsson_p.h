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

#ifndef VENDOR_ERICSSON_P_H
#define VENDOR_ERICSSON_P_H

#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>
#include <qmodemsimtoolkit.h>
#include <qbandselection.h>

class QValueSpaceObject;
class QTimer;
class EricssonModemService;

class EricssonCallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    EricssonCallProvider( QModemService *service );
    ~EricssonCallProvider();

protected:
    QModemCallProvider::AtdBehavior atdBehavior() const;

private slots:
    void ecavNotification( const QString& msg );
};

class TrolltechSimToolkit : public QModemSimToolkit
{
    Q_OBJECT
public:
    TrolltechSimToolkit( QModemService *service );
    ~TrolltechSimToolkit();

public slots:
    void initialize();
    void begin();
    void end();

private slots:
    void tstb( bool ok );
    void tcmd( const QString& value );
    void tcc( const QString& value );
};

class TrolltechBandSelection : public QBandSelection
{
    Q_OBJECT
public:
    TrolltechBandSelection( EricssonModemService *service );
    ~TrolltechBandSelection();

public slots:
    void requestBand();
    void requestBands();
    void setBand( QBandSelection::BandMode mode, const QString& value );

private slots:
    void tbandGet( bool ok, const QAtResult& result );
    void tbandSet( bool ok, const QAtResult& result );
    void tbandList( bool ok, const QAtResult& result );

private:
    EricssonModemService *service;
};

class EricssonModemService : public QModemService
{
    Q_OBJECT
public:
    EricssonModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent = 0, bool trolltechExtensions = false );
    ~EricssonModemService();

    void initialize();

private slots:
    void signalStrength( const QString& msg );
    void smsMemoryFull( const QString& msg );
    void reset();

private:
    bool trolltechExtensions;
};

#endif
