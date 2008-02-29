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

#ifndef QDBUSEXTRATYPES_H
#define QDBUSEXTRATYPES_H

// define some useful types for D-BUS

#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>
#include <QtDBus/qdbusmacros.h>

QT_BEGIN_HEADER

class QDBUS_EXPORT QDBusObjectPath : private QString
{
public:
    inline QDBusObjectPath() { }

    inline explicit QDBusObjectPath(const char *path);
    inline explicit QDBusObjectPath(const QLatin1String &path);
    inline explicit QDBusObjectPath(const QString &path);

    inline void setPath(const QString &path);

    inline QString path() const
    { return *this; }

private:
    void check();
};
Q_DECLARE_METATYPE(QDBusObjectPath)

inline QDBusObjectPath::QDBusObjectPath(const char *objectPath)
    : QString(QString::fromLatin1(objectPath))
{ check(); }

inline QDBusObjectPath::QDBusObjectPath(const QLatin1String &objectPath)
    : QString(objectPath)
{ check(); }

inline QDBusObjectPath::QDBusObjectPath(const QString &objectPath)
    : QString(objectPath)
{ check(); }

inline void QDBusObjectPath::setPath(const QString &objectPath)
{ QString::operator=(objectPath); }


class QDBUS_EXPORT QDBusSignature : private QString
{
public:
    inline QDBusSignature() { }

    inline explicit QDBusSignature(const char *signature);
    inline explicit QDBusSignature(const QLatin1String &signature);
    inline explicit QDBusSignature(const QString &signature);

    inline void setSignature(const QString &signature);

    inline QString signature() const
    { return *this; }

private:
    void check();
};
Q_DECLARE_METATYPE(QDBusSignature)

inline QDBusSignature::QDBusSignature(const char *dBusSignature)
    : QString(QString::fromAscii(dBusSignature))
{ check(); }

inline QDBusSignature::QDBusSignature(const QLatin1String &dBusSignature)
    : QString(dBusSignature)
{ check(); }

inline QDBusSignature::QDBusSignature(const QString &dBusSignature)
    : QString(dBusSignature)
{ check(); }

inline void QDBusSignature::setSignature(const QString &dBusSignature)
{ QString::operator=(dBusSignature); }

class QDBusVariant : private QVariant
{
public:
    inline QDBusVariant() { }
    inline explicit QDBusVariant(const QVariant &variant);

    inline void setVariant(const QVariant &variant);

    inline QVariant variant() const
    { return *this; }
};
Q_DECLARE_METATYPE(QDBusVariant)

inline  QDBusVariant::QDBusVariant(const QVariant &dBusVariant)
    : QVariant(dBusVariant) { }

inline void QDBusVariant::setVariant(const QVariant &dBusVariant)
{ QVariant::operator=(dBusVariant); }


QT_END_HEADER

#endif
