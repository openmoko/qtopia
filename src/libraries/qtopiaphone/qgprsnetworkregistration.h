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

#ifndef QGPRSNETWORKREGISTRATION_H
#define QGPRSNETWORKREGISTRATION_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <QList>

class QTOPIAPHONE_EXPORT QGprsNetworkRegistration : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(QTelephony::RegistrationState registrationState READ registrationState)
    Q_PROPERTY(int locationAreaCode READ locationAreaCode)
    Q_PROPERTY(int cellId READ cellId)
public:
    explicit QGprsNetworkRegistration( const QString& service = QString(),
                                       QObject *parent = 0,
                                       QCommInterface::Mode mode = Client );
    ~QGprsNetworkRegistration();

    QTelephony::RegistrationState registrationState() const;
    int locationAreaCode() const;
    int cellId() const;

signals:
    void registrationStateChanged();
    void locationChanged();
};

class QTOPIAPHONE_EXPORT QGprsNetworkRegistrationServer : public QGprsNetworkRegistration
{
    Q_OBJECT
public:
    explicit QGprsNetworkRegistrationServer
            ( const QString& service, QObject *parent = 0 );
    ~QGprsNetworkRegistrationServer();

protected:
    void updateRegistrationState( QTelephony::RegistrationState state );
    void updateRegistrationState
        ( QTelephony::RegistrationState state,
          int locationAreaCode, int cellId );
};

#endif /* QGPRSNETWORKREGISTRATION_H */
