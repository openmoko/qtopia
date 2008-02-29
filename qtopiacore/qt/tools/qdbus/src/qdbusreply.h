/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDBUSREPLY_H
#define QDBUSREPLY_H

#include <QtCore/qglobal.h>
#include <QtCore/qvariant.h>

#include <QtDBus/qdbusmacros.h>
#include <QtDBus/qdbusmessage.h>
#include <QtDBus/qdbuserror.h>
#include <QtDBus/qdbusextratypes.h>

QT_BEGIN_HEADER


QDBUS_EXPORT void qDBusReplyFill(const QDBusMessage &reply, QDBusError &error, QVariant &data);

template<typename T>
class QDBusReply
{
    typedef T Type;
public:
    inline QDBusReply(const QDBusMessage &reply)
    {
        *this = reply;
    }
    inline QDBusReply& operator=(const QDBusMessage &reply)
    {
        QVariant data(qMetaTypeId(&m_data), reinterpret_cast<void*>(0));
        qDBusReplyFill(reply, m_error, data);
        m_data = qvariant_cast<Type>(data);
        return *this;
    }

    inline QDBusReply(const QDBusError &dbusError = QDBusError())
        : m_error(dbusError), m_data(Type())
    {
    }
    inline QDBusReply& operator=(const QDBusError& dbusError)
    {
        m_error = dbusError;
        m_data = Type();
        return *this;
    }

    inline QDBusReply& operator=(const QDBusReply& other)
    {
        m_error = other.m_error;
        m_data = other.m_data;
        return *this;
    }

    inline bool isValid() const { return !m_error.isValid(); }

    inline const QDBusError& error() { return m_error; }

    inline Type value() const
    {
        return m_data;
    }

    inline operator Type () const
    {
        return m_data;
    }

private:
    QDBusError m_error;
    Type m_data;
};

# ifndef Q_QDOC
// specialize for QVariant:
template<> inline QDBusReply<QVariant>&
QDBusReply<QVariant>::operator=(const QDBusMessage &reply)
{
    void *null = 0;
    QVariant data(qMetaTypeId<QDBusVariant>(), null);
    qDBusReplyFill(reply, m_error, data);
    m_data = qvariant_cast<QDBusVariant>(data).variant();
    return *this;
}

// specialize for void:
template<>
class QDBusReply<void>
{
public:
    inline QDBusReply(const QDBusMessage &reply)
        : m_error(reply)
    {
    }
    inline QDBusReply(const QDBusError &dbusError)
        : m_error(dbusError)
    {
    }

    inline bool isValid() const { return !m_error.isValid(); }

    inline const QDBusError& error() { return m_error; }

private:
    QDBusError m_error;
};
# endif

QT_END_HEADER

#endif
