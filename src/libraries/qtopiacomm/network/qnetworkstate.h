/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#ifndef QNETWORK_STATE
#define QNETWORK_STATE

#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

#if QT_VERSION < 0x040200
#include <QHostAddress>
#else
#include <QNetworkInterface>
#endif
#include <QList>
#include <QString>
#include <QObject>

class QNetworkStatePrivate;

class QTOPIACOMM_EXPORT QNetworkState : public QObject
{
    Q_OBJECT
public:
    explicit QNetworkState( QObject* parent = 0 );
    virtual ~QNetworkState();

    QString gateway() const;
    QList<QString> interfacesOnline() const;

    QString defaultWapAccount() const;

    static QList<QString> availableNetworkDevices( QtopiaNetwork::Type type = QtopiaNetwork::Any);
    static QtopiaNetwork::Type deviceType( const QString& devHandle );

Q_SIGNALS:
    void defaultGatewayChanged( QString devHandle, const QNetworkInterface& localAddress );
    void connected();
    void disconnected();

private Q_SLOTS:
    void gatewayChanged( const QString& newGateway );

private:
    QNetworkStatePrivate* d;
};

#endif
