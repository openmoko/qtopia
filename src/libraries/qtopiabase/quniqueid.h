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

#ifndef QTOPIAID_H
#define QTOPIAID_H

#include <qtopiaglobal.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>
#include <QString>
#include <QUuid>

class QUniqueIdGenerator;
class QTOPIA_EXPORT QUniqueId
{
    friend class QUniqueIdGenerator;
public:
    QUniqueId();
    QUniqueId(const QUniqueId &o);
    explicit QUniqueId(const QString &s);
    explicit QUniqueId(const QByteArray &);
    explicit QUniqueId(uint);

    uint index() const;
    QUuid context() const;
    QUuid device() const;

    QUniqueId operator=(const QUniqueId &o);

    bool operator==(const QUniqueId &o) const
    { return mId == o.mId && mContext == o.mContext; }
    bool operator!=(const QUniqueId &o) const
    { return mId != o.mId || mContext != o.mContext; }

    // these are not really useful, and could be harmful.
    bool operator<(QUniqueId o) const
        { return mContext == o.mContext ? mId < o.mId : mContext < o.mContext; }
    bool operator>(QUniqueId o) const
        { return mContext == o.mContext ? mId > o.mId : mContext > o.mContext; }
    bool operator<=(QUniqueId o) const { return *this == o || *this < o; }
    bool operator>=(QUniqueId o) const { return *this == o || *this > o; }

    bool isNull() const { return mId == 0; }

    bool isTemporary() const;

    QString toString() const;
    QByteArray toByteArray() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

protected:
    // automatically reversable via constructors, which can detect format.
    QString toLocalContextString() const;
    QByteArray toLocalContextByteArray() const;
#ifndef QT_NO_DATASTREAM
    QDataStream &fromLocalContextDataStream( QDataStream & );
    QDataStream &toLocalContextDataStream( QDataStream & ) const;
#endif

private:
    uint mContext; // covers both device and context.
    uint mId;

    static QUuid deviceId();
    static QUuid legacyIdContext();
    static QUuid temporaryIDContext();

    friend uint qHash(const QUniqueId &uid);
};

inline uint qHash(const QUniqueId &uid) { return uid.mId; }

class QTOPIA_EXPORT QLocalUniqueId : public QUniqueId
{
public:
    QLocalUniqueId() : QUniqueId() {}
    QLocalUniqueId(const QUniqueId &o) : QUniqueId(o) {}
    QLocalUniqueId(const QLocalUniqueId &o) : QUniqueId(o) {}
    QLocalUniqueId(const QString &s) : QUniqueId(s) {}
    QLocalUniqueId(const QByteArray &b) : QUniqueId(b) {}

    QString toString() const { return toLocalContextString(); }
    QByteArray toByteArray() const { return toLocalContextByteArray(); }

#ifndef QT_NO_DATASTREAM
    QDataStream &fromLocalContextDataStream( QDataStream &s )
        { return QUniqueId::fromLocalContextDataStream(s); }
    QDataStream &toLocalContextDataStream( QDataStream &s ) const
        { return QUniqueId::toLocalContextDataStream(s); }
    friend QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, QUniqueId & );
    friend QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const QUniqueId & );
#endif
};


class QTOPIA_EXPORT QUniqueIdGenerator
{
public:
    QUniqueIdGenerator(const QUuid &context);
    QUniqueIdGenerator(const QUuid &device, const QUuid &context);
    QUniqueIdGenerator(const QUniqueIdGenerator &other);

    ~QUniqueIdGenerator();

    bool isValid() const { return mContext != 0; }
    QUniqueId createUniqueId();

    QUniqueId constructUniqueId(uint);

    static QUniqueId temporaryID(uint);

private:
    uint mContext; // a combination of device mapped and scope mapped context.
    uint mLastId;
    uint mReserved;
};

Q_DECLARE_USER_METATYPE(QUniqueId)

#endif
