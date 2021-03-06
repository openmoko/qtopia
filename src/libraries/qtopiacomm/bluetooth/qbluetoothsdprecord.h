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

#ifndef __QBLUETOOTHSDPRECORD_H__
#define __QBLUETOOTHSDPRECORD_H__

#include <QMap>
#include <QVariant>
#include <QList>
#include <qglobal.h>

#include <qbluetoothglobal.h>
#include <qbluetoothnamespace.h>

class QString;
class QBluetoothSdpUuid;

class QBLUETOOTH_EXPORT QBluetoothSdpAlternative : public QList<QVariant>
{

};

class QBLUETOOTH_EXPORT QBluetoothSdpSequence : public QList<QVariant>
{

};

class QBLUETOOTH_EXPORT QBluetoothSdpRecord {
public:
    static int rfcommChannel(const QBluetoothSdpRecord &service);
    static QBluetoothSdpRecord fromDevice(QIODevice *device);
    static QBluetoothSdpRecord fromData(const QByteArray &data);

    QBluetoothSdpRecord();
    ~QBluetoothSdpRecord();

    bool isNull() const;

    QBluetoothSdpRecord(const QBluetoothSdpRecord &other);
    QBluetoothSdpRecord &operator=(const QBluetoothSdpRecord &other);
    bool operator==(const QBluetoothSdpRecord &other) const;

    bool isInstance(QBluetooth::SDPProfile profile) const;
    bool isInstance(const QBluetoothSdpUuid &uuid) const;

    QString serviceName() const;
    void setServiceName(const QString &serviceName);

    QString serviceDescription() const;
    void setServiceDescription(const QString &serviceDesc);

    QString providerName() const;
    void setProviderName(const QString &providerName);

    QUrl docUrl() const;
    void setDocUrl(const QUrl &docUrl);

    QUrl execUrl() const;
    void setExecUrl(const QUrl &execUrl);

    QUrl iconUrl() const;
    void setIconUrl(const QUrl &iconUrl);

    QBluetoothSdpUuid id() const;
    void setId(const QBluetoothSdpUuid &id);

    QList<QBluetoothSdpUuid> browseGroups() const;
    void setBrowseGroups(const QList<QBluetoothSdpUuid> &groups);

    QBluetoothSdpUuid group() const;
    void setGroup(const QBluetoothSdpUuid &group);

    quint32 recordHandle() const;
    void setRecordHandle(quint32 handle);

    QList<quint16> attributeIds() const;
    bool addAttribute(quint16 id, const QVariant &attr);
    bool removeAttribute(quint16 id);
    QVariant attribute(quint16 id, bool *ok = NULL) const;
    void clearAttributes();

private:
    QMap<quint16, QVariant> m_attrs;
};

Q_DECLARE_METATYPE(QBluetoothSdpSequence)
Q_DECLARE_METATYPE(QBluetoothSdpAlternative)

#endif
