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

#ifndef __SDAP_H__
#define __SDAP_H__

#include <qsdpservice.h>
#include <qsdpuuid.h>
#include <qbluetoothnamespace.h>

#include <QString>
#include <QList>
#include <QtCore/qglobal.h>
#include <qobject.h>

class QBluetoothAddress;
class QBluetoothLocalDevice;
class QSDPUUID;

class QSDAP_Private;

class QTOPIACOMM_EXPORT QSDAPSearchResult
{
public:
    QSDAPSearchResult();

    QSDAPSearchResult(const QSDAPSearchResult &other);
    QSDAPSearchResult &operator=(const QSDAPSearchResult &other);

    bool errorOccurred() const;
    void clearError();

    QString error() const;
    void setError(const QString &error);

    const QList<QSDPService> &services() const;
    void addService(const QSDPService &service);
    void clear();

private:
    QList<QSDPService> m_services;
    QString m_error;
};

class QTOPIACOMM_EXPORT QSDAP : public QObject
{
    Q_OBJECT

public:
    QSDAP();
    ~QSDAP();

    bool searchServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local,
                        QBluetooth::SDPProfile profile);
    bool searchServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local,
                        const QSDPUUID &uuid);

    bool searchLocalServices(const QBluetoothLocalDevice &local,
                             QBluetooth::SDPProfile profile);
    bool searchLocalServices(const QBluetoothLocalDevice &local,
                             const QSDPUUID &uuid);

    bool browseServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local);

    bool browseLocalServices(const QBluetoothLocalDevice &local);

public slots:
    void cancelSearch();

signals:
    void searchComplete(const QSDAPSearchResult &result);

private slots:
    void processFinished();
    void readServices();

private:
    QSDAP_Private *m_data;

    Q_DISABLE_COPY(QSDAP)
};

#endif
