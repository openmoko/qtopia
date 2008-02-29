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

#ifndef QNETWORK_DEVICE
#define QNETWORK_DEVICE

#include <qtopianetworkinterface.h>
#include <qtopianetwork.h>
#include <qtopiaglobal.h>

#include <QNetworkInterface>

class QNetworkDevicePrivate;

class QTOPIACOMM_EXPORT QNetworkDevice : public QObject
{
    Q_OBJECT
public:
    explicit QNetworkDevice( const QString& handle, QObject* parent = 0 );
    ~QNetworkDevice();

    QtopiaNetworkInterface::Status state() const;

    QtopiaNetworkInterface::Error error() const;
    QString errorString() const;

    QNetworkInterface address() const;
    QString interfaceName() const;
    QString handle() const;
    QString name() const;

Q_SIGNALS:
    void stateChanged( QtopiaNetworkInterface::Status newState, bool error );
private:
    QNetworkDevicePrivate* d;
};

#endif
