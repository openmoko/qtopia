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

#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <QMap>
#include <QVariant>
#include <QList>
#include <qglobal.h>

#include <qtopiaglobal.h>
#include <qbluetoothnamespace.h>

class QString;
class QSDPUUID;

class QTOPIACOMM_EXPORT QSDPAlternative : public QList<QVariant>
{

};

class QTOPIACOMM_EXPORT QSDPSequence : public QList<QVariant>
{

};

class QTOPIACOMM_EXPORT QSDPService {
public:
    static bool isInstance(const QSDPService &service,
                           QBluetooth::SDPProfile profile);
    static int rfcommChannel(const QSDPService &service);

    QSDPService();
    ~QSDPService();

    QSDPService(const QSDPService &other);
    QSDPService &operator=(const QSDPService &other);

    QString serviceName() const;
    void setServiceName(const QString &serviceName);

    QString serviceDescription() const;
    void setServiceDescription(const QString &serviceDesc);

    QString providerName() const;
    void setProviderName(const QString &providerName);

    QUrl docURL() const;
    void setDocURL(const QUrl &docURL);

    QUrl execURL() const;
    void setExecURL(const QUrl &execURL);

    QUrl iconURL() const;
    void setIconURL(const QUrl &iconURL);

    QSDPUUID id() const;
    void setId(const QSDPUUID &id);

    QList<QSDPUUID> browseGroups() const;
    void setBrowseGroups(const QList<QSDPUUID> &groups);

    QSDPUUID group() const;
    void setGroup(const QSDPUUID &group);

    quint32 recordHandle() const;
    void setRecordHandle(quint32 handle);

    QList<quint16> attributeIds() const;
    bool addAttribute(quint16 id, const QVariant &attr);
    bool removeAttribute(quint16 id);
    QVariant attribute(quint16 id, bool *ok = NULL) const;

private:
    bool operator==(const QSDPService &other) const;
    QMap<quint16, QVariant> m_attrs;
};

Q_DECLARE_METATYPE(QSDPSequence)
Q_DECLARE_METATYPE(QSDPAlternative)

#endif
