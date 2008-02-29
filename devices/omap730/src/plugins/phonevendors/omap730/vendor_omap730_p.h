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

#ifndef	VENDOR_OMAP730_P_H
#define	VENDOR_OMAP730_P_H

#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>
#include <qmodemsimtoolkit.h>
#include <qmodemphonebook.h>
#include <qmodempinmanager.h>


class Omap730CallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    Omap730CallProvider( QModemService *service );
    ~Omap730CallProvider();

protected:
    QModemCallProvider::AtdBehavior atdBehavior() const;
    void abortDial( uint modemIdentifier, QPhoneCall::Scope scope );

private slots:
    void cpiNotification( const QString& msg );
    void cnapNotification( const QString& msg );
};

class Omap730SimToolkit : public QModemSimToolkit
{
    Q_OBJECT
public:
    Omap730SimToolkit( QModemService *service );
    ~Omap730SimToolkit();

public slots:
    void initialize();
    void begin();
    void sendResponse( const QSimTerminalResponse& resp );
    void sendEnvelope( const QSimEnvelope& env );

private slots:
    void satiNotification( const QString& msg );
    void satnNotification( const QString& msg );

private:
    QSimCommand lastCommand;
    QByteArray lastCommandBytes;
    QSimCommand mainMenu;
    QByteArray mainMenuBytes;
    bool lastResponseWasExit;
};

class Omap730PhoneBook : public QModemPhoneBook
{
    Q_OBJECT
public:
    Omap730PhoneBook( QModemService *service );
    ~Omap730PhoneBook();

protected:
    bool hasEmptyPhoneBookIndex() const;
};

class Omap730PinManager : public QModemPinManager
{
    Q_OBJECT
public:
    Omap730PinManager( QModemService *service );
    ~Omap730PinManager();

protected:
    bool emptyPinIsReady() const;
};

class Omap730ModemService : public QModemService
{
    Q_OBJECT
public:
    Omap730ModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent = 0 );
    ~Omap730ModemService();

    void initialize();

private slots:
    void csq( const QString& msg );
    void configureDone( bool ok );
    void reset();
};

#endif
