/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDBUSABSTRACTINTERFACE_H
#define QDBUSABSTRACTINTERFACE_H

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>

#include <QtDBus/qdbusmessage.h>
#include <QtDBus/qdbusextratypes.h>
#include <QtDBus/qdbusconnection.h>

QT_BEGIN_HEADER

class QDBusError;

class QDBusAbstractInterfacePrivate;
class QDBUS_EXPORT QDBusAbstractInterface: public QObject
{
    Q_OBJECT

public:
    virtual ~QDBusAbstractInterface();
    bool isValid() const;

    QDBusConnection connection() const;

    QString service() const;
    QString path() const;
    QString interface() const;

    QDBusError lastError() const;

    QDBusMessage call(const QString &method,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());

    QDBusMessage call(QDBus::CallMode mode,
                      const QString &method,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());

    QDBusMessage callWithArgumentList(QDBus::CallMode mode,
                                      const QString &method,
                                      const QList<QVariant> &args);

    bool callWithCallback(const QString &method,
                          const QList<QVariant> &args,
                          QObject *receiver, const char *member);

protected:
    QDBusAbstractInterface(const QString &service, const QString &path, const char *interface,
                           const QDBusConnection &connection, QObject *parent);
    QDBusAbstractInterface(QDBusAbstractInterfacePrivate &, QObject *parent);

    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
    QVariant internalPropGet(const char *propname) const;
    void internalPropSet(const char *propname, const QVariant &value);
    QDBusMessage internalConstCall(QDBus::CallMode mode,
                                   const QString &method,
                                   const QList<QVariant> &args = QList<QVariant>()) const;

private:
    Q_DECLARE_PRIVATE(QDBusAbstractInterface)
    Q_PRIVATE_SLOT(d_func(), void _q_serviceOwnerChanged(QString,QString,QString))
};

QT_END_HEADER

#endif
