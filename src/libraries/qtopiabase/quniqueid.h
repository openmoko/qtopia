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

    uint index() const;
    QUuid context() const;

    uint mappedContext() const;

    QUniqueId operator=(const QUniqueId &o);

    bool operator==(const QUniqueId &o) const { return mId == o.mId; }
    bool operator!=(const QUniqueId &o) const { return mId != o.mId; }

    // these are not really useful, and could be harmful.
    bool operator<(QUniqueId o) const { return mId < o.mId; }
    bool operator>(QUniqueId o) const { return mId > o.mId; }
    bool operator<=(QUniqueId o) const { return mId <= o.mId; }
    bool operator>=(QUniqueId o) const { return mId >= o.mId; }

    bool isNull() const { return mId == 0; }

    bool isTemporary() const;

    QString toString() const;
    QByteArray toByteArray() const;

    uint toUInt() const;
    static QUniqueId fromUInt(uint);

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    QString toLocalContextString() const;
    QByteArray toLocalContextByteArray() const;

protected:
    // automatically reversable via constructors, which can detect format.
#ifndef QT_NO_DATASTREAM
    QDataStream &fromLocalContextDataStream( QDataStream & );
    QDataStream &toLocalContextDataStream( QDataStream & ) const;
#endif

private:
    void setContext(uint context);
    void setIndex(uint index);
    void setIdentity(uint context, uint index);
    uint mId;

    static QUuid legacyIdContext();
    static QUuid temporaryIdContext();

    friend uint qHash(const QUniqueId &uid);
};

inline uint qHash(const QUniqueId &uid) { return uid.mId; }

class QTOPIA_EXPORT QUniqueIdGenerator
{
public:
    QUniqueIdGenerator(const QUuid &context);
    QUniqueIdGenerator(const QUniqueIdGenerator &other);

    ~QUniqueIdGenerator();

    bool isValid() const { return mContext != 0; }
    QUniqueId createUniqueId();

    QUniqueId constructUniqueId(uint);

    static QUniqueId constructTemporaryId(uint);
    static QUniqueId createTemporaryId();

    static uint mappedContext(const QUuid &context);
private:
    uint mContext; // a combination of device mapped and scope mapped context.
    uint mLastId;
    uint mReserved;
};

Q_DECLARE_USER_METATYPE(QUniqueId)

#endif
