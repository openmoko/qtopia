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

#ifndef QDBUSMESSAGE_P_H
#define QDBUSMESSAGE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <qatomic.h>
#include <qstring.h>
struct DBusMessage;

class QDBusMessagePrivate
{
public:
    QDBusMessagePrivate();
    ~QDBusMessagePrivate();

    QList<QVariant> arguments;
    QString service, path, interface, name, message, signature;
    DBusMessage *msg;
    DBusMessage *reply;
    int type;
    int timeout;
    QAtomic ref;

    uint delayedReply : 1;
    uint localMessage : 1;

    static DBusMessage *toDBusMessage(const QDBusMessage &message);
    static QDBusMessage fromDBusMessage(DBusMessage *dmsg);
    static QDBusMessage fromError(const QDBusError& error);
    static QDBusMessage updateSignature(const QDBusMessage &message, DBusMessage *dmsg);

    static void setLocal(const QDBusMessage *message, bool local);
    static bool isLocal(const QDBusMessage &message);
    static void setArguments(const QDBusMessage *message, const QList<QVariant> &arguments);
    static void setType(const QDBusMessage *message, QDBusMessage::MessageType type);
};

#endif
