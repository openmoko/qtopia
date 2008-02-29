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

#ifndef __QBLUETOOTHABSTRACTSERVICE_H__
#define __QBLUETOOTHABSTRACTSERVICE_H__

#include <qbluetoothnamespace.h>

class QBluetoothAddress;
class QBluetoothSdpRecord;
class QBluetoothAbstractServicePrivate;

class QBLUETOOTH_EXPORT QBluetoothAbstractService : public QObject
{
    Q_OBJECT

public:
    explicit QBluetoothAbstractService(const QString &name, const QString &displayName, QObject *parent = 0);
    virtual ~QBluetoothAbstractService();

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setSecurityOptions(QBluetooth::SecurityOptions options) = 0;

    QString name() const;
    QString displayName() const;

protected:
    quint32 registerRecord(const QBluetoothSdpRecord &record);
    quint32 registerRecord(const QString &filename);
    bool unregisterRecord(quint32 handle);

private:
    QBluetoothAbstractServicePrivate *m_data;

signals:
    void started(bool error, const QString &description);
    void stopped();
};

#endif
