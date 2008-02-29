/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSINTERFACEPRIVATE_H
#define QDBUSINTERFACEPRIVATE_H

#include <qdbusabstractinterface_p.h>
#include <qdbusmetaobject_p.h>
#include <qdbusinterface.h>

class QDBusInterfacePrivate: public QDBusAbstractInterfacePrivate
{
public:
    Q_DECLARE_PUBLIC(QDBusInterface)

    QDBusMetaObject *metaObject;

    QDBusInterfacePrivate(const QString &serv, const QString &p, const QString &iface,
                          const QDBusConnection &con);
    ~QDBusInterfacePrivate();

    int metacall(QMetaObject::Call c, int id, void **argv);
};

#endif
