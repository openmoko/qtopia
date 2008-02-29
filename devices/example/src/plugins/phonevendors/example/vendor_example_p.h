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

#ifndef VENDOR_EXAMPLE_P_H
#define VENDOR_EXAMPLE_P_H

#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>

class QValueSpaceObject;
class QTimer;


class ExampleCallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    ExampleCallProvider( QModemService *service );
    ~ExampleCallProvider();

protected:
    QModemCallProvider::AtdBehavior atdBehavior() const;
    QString putOnHoldCommand() const;  
};

class ExampleModemService : public QModemService
{
    Q_OBJECT
public:
    ExampleModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent = 0 );
    ~ExampleModemService();

    void initialize();

private slots:
    void signalStrength( const QString& msg );
    void reset();
};

#endif
