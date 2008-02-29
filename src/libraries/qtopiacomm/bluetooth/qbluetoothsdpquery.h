/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qbluetoothsdprecord.h>
#include <qbluetoothsdpuuid.h>
#include <qbluetoothnamespace.h>

#include <QString>
#include <QList>
#include <QtCore/qglobal.h>
#include <qobject.h>

class QBluetoothAddress;
class QBluetoothLocalDevice;
class QBluetoothSdpUuid;

class QBluetoothSdpQuery_Private;

class QTOPIACOMM_EXPORT QBluetoothSdpQueryResult
{
public:
    QBluetoothSdpQueryResult();

    QBluetoothSdpQueryResult(const QBluetoothSdpQueryResult &other);
    QBluetoothSdpQueryResult &operator=(const QBluetoothSdpQueryResult &other);

    bool isValid() const;

    void setError(const QString &error);
    QString error() const;

    const QList<QBluetoothSdpRecord> &services() const;
    void addService(const QBluetoothSdpRecord &service);

    void reset();

private:
    QList<QBluetoothSdpRecord> m_services;
    QString m_error;
    bool m_valid;
};

class QTOPIACOMM_EXPORT QBluetoothSdpQuery : public QObject
{
    Q_OBJECT

    friend class QBluetoothSdpQuery_Private;

public:
    QBluetoothSdpQuery( QObject* parent = 0 );
    ~QBluetoothSdpQuery();

    bool searchServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local,
                        QBluetooth::SDPProfile profile);
    bool searchServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local,
                        const QBluetoothSdpUuid &uuid);
    bool browseServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local);

public slots:
    void cancelSearch();

signals:
    void searchComplete(const QBluetoothSdpQueryResult &result);
    void searchCancelled();

private:
    QBluetoothSdpQuery_Private *m_data;
    Q_DISABLE_COPY(QBluetoothSdpQuery)
};

#endif
