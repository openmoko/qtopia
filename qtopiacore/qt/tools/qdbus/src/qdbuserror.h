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

#ifndef QDBUSERROR_H
#define QDBUSERROR_H

#include <QtDBus/qdbusmacros.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

struct DBusError;
class QDBusMessage;

class QDBUS_EXPORT QDBusError
{
public:
    enum ErrorType {
        NoError = 0,
        Other = 1,
        Failed,
        NoMemory,
        ServiceUnknown,
        NoReply,
        BadAddress,
        NotSupported,
        LimitsExceeded,
        AccessDenied,
        NoServer,
        Timeout,
        NoNetwork,
        AddressInUse,
        Disconnected,
        InvalidArgs,
        UnknownMethod,
        TimedOut,
        InvalidSignature,
        UnknownInterface,
        InternalError,

#ifndef Q_QDOC
        // don't use this one!
        LastErrorType = InternalError
#endif
    };

    QDBusError(const DBusError *error = 0);
    QDBusError(const QDBusMessage& msg);
    QDBusError(ErrorType error, const QString &message);
    QDBusError(const QDBusError &other);
    QDBusError &operator=(const QDBusError &other);

    ErrorType type() const;
    QString name() const;
    QString message() const;
    bool isValid() const;

private:
    ErrorType code;
    QString msg;
    QString nm;
    void *unused;
};

#ifndef QT_NO_DEBUG_STREAM
QDBUS_EXPORT QDebug operator<<(QDebug, const QDBusError &);
#endif

QT_END_HEADER

#endif
