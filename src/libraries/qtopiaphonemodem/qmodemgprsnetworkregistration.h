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

#ifndef QMODEMGPRSNETWORKREGISTRATION_H
#define QMODEMGPRSNETWORKREGISTRATION_H

#include <qgprsnetworkregistration.h>

class QModemGprsNetworkRegistrationPrivate;
class QAtResult;
class QModemService;

class QTOPIAPHONEMODEM_EXPORT QModemGprsNetworkRegistration
        : public QGprsNetworkRegistrationServer
{
    Q_OBJECT
public:
    explicit QModemGprsNetworkRegistration( QModemService *service );
    ~QModemGprsNetworkRegistration();

private slots:
    void resetModem();
    void cgregQuery( bool ok, const QAtResult& result );
    void cgregNotify( const QString& msg );

private:
    QModemGprsNetworkRegistrationPrivate *d;
};

#endif /* QMODEMGPRSNETWORKREGISTRATION_H */
