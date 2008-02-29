/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "quniqueid.h"

#include <qtopianamespace.h>
#include <qtopialog.h>

#include <QSettings>
#include <QFile>
#include <QCache>
#include <QUuid>
#include <QDataStream>
#include <QDebug>

class QUniqueIdTracker
{
public:
    QUniqueIdTracker(const QString &filename);

    uint context(const QUuid &left, const QUuid &right) const;

    QUuid left(uint context) const;
    QUuid right(uint context) const;

    // returns next index starting at reserve spot.
    uint reserveIndex(uint context, uint count);
    uint reserveContext(const QUuid &left, const QUuid &right);

private:
    struct IdState {
        IdState() : nextId(1) {}
        IdState(const IdState &other)
            : left(other.left), right(other.right), nextId(other.nextId) {}
        QUuid left;
        QUuid right;
        uint nextId;
    };

    IdState *getItem(uint) const;
    uint getItem(const QUuid &left, const QUuid &right) const;

    bool compare(const IdState *, const QUuid &left, const QUuid &right);

    void read(IdState *item, QIODevice &dev) const
    {
        QDataStream ds(&dev);
        ds >> item->left;
        ds >> item->right;
        ds >> item->nextId;
    }
    void write(const IdState *item, QIODevice &dev) const
    {
        QDataStream ds(&dev);
        ds << item->left;
        ds << item->right;
        ds << item->nextId;
    }

    void read(uint &i, QIODevice &dev) const
    {
        QDataStream ds(&dev);
        ds >> i;
    }

    void write(uint i, QIODevice &dev) const
    {
        QDataStream ds(&dev);
        ds << i;
    }

    mutable QCache<uint, IdState> cache;
    mutable QFile file;
};

static QUniqueIdTracker *result = 0;

static void clean_uniqueIDTracker()
{
    if ( result )
        delete result;
    result = 0;
}

static QUniqueIdTracker &uniqueIdTracker()
{
    if (!result) {
        result = new QUniqueIdTracker(Qtopia::applicationFileName("Qtopia", "QUniqueIdTrackerFile"));
        qAddPostRoutine( clean_uniqueIDTracker );
    }
    return *result;
}

// TODO should add some basic ipc.
QUniqueIdTracker::QUniqueIdTracker(const QString &filename)
{
    //header is 16 (mkey) + 4 (version) + 4 (nextoncext) = 24 bytes.
    static QByteArray mkey("QUniqueIdTraker."); // 16 bytes.
    file.setFileName(filename);

    if (file.exists()) {
        // read state
        if (!file.open(QIODevice::ReadOnly))
            qFatal("Could not open id tracker.  should wait on this");
        // check its the right file
        if (mkey != file.read(16))
            qFatal("Invalid key at start if id tracker.");
        // get version.
        uint version, nextcontext;
        read(version, file);
        read(nextcontext, file);
        if (version != 1) // current version
            qFatal("Invalid version id tracker file.");
        if (nextcontext == 0xffffffff)
            qFatal("Ids full"); // this device must have got a lot of use.
        // want a non-locking hold open.  memory file with changing size?
        file.close();
    } else {
        // init state
        file.open(QIODevice::WriteOnly);
        file.write(mkey, 16);
        write(1, file);
        write(1, file);
        // 36-24 = 12
        QByteArray nullData(12, 0);
        file.write(nullData);
        file.close();
    }
}

uint QUniqueIdTracker::context(const QUuid &left, const QUuid &right) const
{
    return getItem(left, right);
}

QUuid QUniqueIdTracker::left(uint context) const
{
    IdState *item = getItem(context);
    if (!item)
        return QUuid();
    return item->left;
}

QUuid QUniqueIdTracker::right(uint context) const
{
    IdState *item = getItem(context);
    if (!item)
        return QUuid();
    return item->right;
}

uint QUniqueIdTracker::reserveIndex(uint context, uint count)
{
    IdState *item = getItem(context); // also checks validity of context.
    if (!item)
        return 0; // no context exists.
    // TODO ipc and locking of file.
    file.open(QIODevice::ReadWrite);
    file.seek(36*context); // first context is 1.  header is at 0 and same size as items

    read(item, file); // refresh item.
    // shoudl do an overflow check.
    item->nextId += count;
    file.seek(36*context);
    write(item, file);

    file.close();
    return item->nextId - count;
}

uint QUniqueIdTracker::reserveContext(const QUuid &left, const QUuid &right)
{
    if (left.isNull() || right.isNull())
        return 0;

    uint context = getItem(left, right);
    if (context)
        return context;
    // create a context.
    file.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    file.seek(20);
    quint32 nextcontext;
    read(nextcontext, file);

    IdState *item = new IdState();
    item->left = left;
    item->right = right;
    item->nextId = 1;
    cache.insert(nextcontext, item);

    if(36*nextcontext > file.size()) {
        file.close();
        file.open(QIODevice::Append | QIODevice::Unbuffered);
    }
    else
        file.seek(36*nextcontext);

    write(item, file);

    file.seek(20);
    write(nextcontext+1, file);

    file.close();
    return nextcontext;
}

QUniqueIdTracker::IdState *QUniqueIdTracker::getItem(uint context) const
{
    if (!context)
        return 0;
    if (cache.contains(context))
        return cache.object(context);

    // assume have to read.
    file.open(QIODevice::ReadOnly);
    file.seek(20);
    quint32 nextcontext;
    read(nextcontext, file);
    if (context >= nextcontext)
        return 0;
    file.seek(36*context);
    IdState *item = new IdState();
    read(item, file);
    file.close();
    cache.insert(context, item);
    return item;
}

uint QUniqueIdTracker::getItem(const QUuid &left, const QUuid &right) const
{
    // this one much harder (and is reason for cache)
    QList<uint> keys = cache.keys();
    // first try in the cache.
    foreach(uint k, keys) {
        if (cache[k]->left == left && cache[k]->right == right)
            return k;
    }
    // not in cache.  now need to do file.  only cache result.
    file.open(QIODevice::ReadOnly);
    quint32 max;
    file.seek(20);
    read(max, file);
    for (uint i = 1; i < max; i++) {
        file.seek(i*36);
        IdState state;
        read(&state, file);
        if (left == state.left && right == state.right) {
            IdState *item = new IdState(state);
            cache.insert(i, item);
            file.close();
            return i;
        }
    }
    file.close();
    return 0;
}

/*!
    \fn void QUniqueId::serialize(Stream &value) const

    \internal

    Serializes the QUniqueId instance out to a template
    type \c{Stream} \a stream.
 */
template <typename Stream> void QUniqueId::serialize(Stream &stream) const
{
    QUuid left = uniqueIdTracker().left(mContext);
    QUuid right = uniqueIdTracker().right(mContext);
    stream << left;
    stream << right;
    stream << mId;
}

/*!
    \fn void QUniqueId::deserialize(Stream &value)

    \internal

    Deserializes the QUniqueId instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QUniqueId::deserialize(Stream &stream)
{
    QUuid left;
    QUuid right;
    stream >> left;
    stream >> right;
    stream >> mId;
    mContext = uniqueIdTracker().reserveContext(left, right);
    if (mContext == 0)
        mId = 0;
}

#ifndef QT_NO_DATASTREAM

/*!
  \overload

  Writes the \a id to the datastream \a s.  Not suitable for data that may be transfered
  off the device.
*/
QDataStream &operator<<( QDataStream &s, const QLocalUniqueId &id )
{
    id.toLocalContextDataStream(s);
    return s;
}

/*!
  \overload

  Reads the \a id from the datastream \a s.  Not suitable for data that may have be transfered
  from another device.
*/
QDataStream &operator>>( QDataStream &s, QLocalUniqueId &id )
{
    id.fromLocalContextDataStream(s);
    return s;
}

#endif

QUuid QUniqueId::legacyIdContext()
{
    return QUuid("35a20342-a280-40d5-a18f-f45cd9d92e7d");
}

QUuid QUniqueId::temporaryIDContext()
{
    return QUuid("f3ed326a-f37a-4aa0-bad3-f7968e9ecd60");
}

QUuid QUniqueId::deviceId() {
    // should change based of each device.  Maybe get from custom.h?
    static QUuid device("802be8cf-25d6-43eb-88ec-1b2204ec668c");

    qLog(Support) << "Get device id" << device.toString();
    return device;
}

/*!
  \class QUniqueIdGenerator
  \brief The QUniqueIdGenerator class provides a generator of unique identifiers.

  The QUniqueIdGenerator class provides a generator of unique identifiers.  These identifiers
  are unique within any Qtopia device but can be stored with only 64 bits locally.  When
  transferring or storing in data that may be transfered to another device the ids are
  expanded to 288 bits. This is achieved by keeping a record of contexts seen.

  Ids are generated with a QUuid Device identifier, a QUuid Context identifer and an index.
  The context identifer should be generated with a program such as \a quuid and represent
  the data store or context within which the id is generated.  For example, XML, SQL , and SIM
  card storage for QContacts each have their own Context Identifier.

  Usually usage will be:

\code
  QUniqueIdGenerator g(myContext());
  id = g.createId();
\endcode

  However, if creating an id from a source that has its own 32 bit (non 0) identifiers, the identifiers
  can be constructed thus avoiding storing the QUniqueIdentifiers on the store.

\code
  QUniqueIdGenerator g(myContext());
  id = g.constructId(index);
\endcode

  If the identifier is for removable media or a foriegn device, a device id should
  also be obtained for the generator.

  \ingroup qtopiaemb
*/

/*!
  \fn bool QUniqueIdGenerator::isValid() const

  Returns true if the generator is valid and can be used to create or construct
  unique id's.  Otherwise returns false.

  This function should only be needed during debugging as only a generator
  constructed using null QUuid's will be invalid.
*/

/*!
  Create a QUniqueIdGenerator with the scope \a context.
*/
QUniqueIdGenerator::QUniqueIdGenerator(const QUuid &context)
    : mLastId(0), mReserved(0)
{
    mContext = uniqueIdTracker().reserveContext(QUniqueId::deviceId(), context);
}

/*!
  Create a QUniqueIdGenerator with the scope \a context as if from \a device.
  Should only be used for importing foriegn data (e.g. from SIM card).
*/
QUniqueIdGenerator::QUniqueIdGenerator(const QUuid &device, const QUuid &context)
    : mLastId(0), mReserved(0)
{
    mContext = uniqueIdTracker().reserveContext(device, context);
}

/*!
  Create a deep copy QtopiaIdGen from \a other.
*/
QUniqueIdGenerator::QUniqueIdGenerator(const QUniqueIdGenerator &other)
    : mContext(other.mContext), mLastId(other.mLastId), mReserved(0)
{
}

/*!
  Destroy QUniqueIdGenerator.
*/
QUniqueIdGenerator::~QUniqueIdGenerator() { }

/*!
  Creates a temporary identify based of \a index.

  Temporary identifiers are scoped so as not to conflict
  with other identifiers, but are not suitable for syncing.

  The main use of a temporaryID is to use other, non-QUniqueId objects within the set
  of QtopiaIDs.
*/
QUniqueId QUniqueIdGenerator::temporaryID(uint index)
{
    // TODO could cache mContext for tempoaryId and legacyId specially.
    QUniqueId i;
    i.mContext = uniqueIdTracker().reserveContext(QUniqueId::deviceId(), QUniqueId::temporaryIDContext());
    i.mId = index;
    return i;
}

/*!
  Generate and return a new identifier.  The identifier will be unique from other ids generated with
  this function.

  \sa constructUniqueId()
*/
QUniqueId QUniqueIdGenerator::createUniqueId()
{
    if (mContext == 0)
        return QUniqueId();

    if (mReserved < 1) {
        mLastId = uniqueIdTracker().reserveIndex(mContext, 4);
        mReserved = 4;
    }


    QUniqueId i;
    i.mContext = mContext;
    i.mId = mLastId;
    mReserved--;
    mLastId++;

    qLog(Support) << "createUniqueId" << i.toString();
    return i;
}

/*!
  Constructs a identifier based of \a index for the context of the generator.
  Does not ensure the constructed id is not in conflict with past or future QUniqueId objects.

  \sa createUniqueId()
*/
QUniqueId QUniqueIdGenerator::constructUniqueId(uint index)
{
    if (mContext == 0)
        return QUniqueId();

    QUniqueId id;
    id.mContext = mContext;
    id.mId = index;
    return id;
}


/*!
  Returns true if is a temporary identifier, otherwise returns false.

  Temporary identifiers are scoped so as not to conflict
  with other identifiers, however, they do not include location information and
  are not sutiable for syncing.

  They are suitable for merging temporary data sets when the ids are not
  intended to be permanent.
*/
bool QUniqueId::isTemporary() const
{
    // faster to look up via context.
    QUuid d = uniqueIdTracker().left(mContext);
    QUuid c = uniqueIdTracker().right(mContext);
    return (d == temporaryIDContext() && c == temporaryIDContext());
}

/*!
  \class QUniqueId
  \brief The QUniqueId class provides an identifer that is unique and yet can be stored
  efficiently.

  The QUniqueId class provides an identifer that is unqique and yet can be stored
  with a small number of bits.  These identifiers
  are unique within any Qtopia device and can be stored locally with only 64 bits.  When
  transferring or storing in data that may be transfered to another device the ids are
  expanded to 288 bits by keeping a record of contexts seen.

  Ids are generated with:
  \list
  \o a QUuid Device identifier
  \o a QUuid Context identifier
  \o an index
  \endlist

  The context identifer should be generated with a program such as \a quuid and represent
  the data store or context within which the id is generated.  For instance XML, SQL, and SIM
  card storage for QContacts each have their own Context Identifier.

  \sa QUniqueIdGenerator

  \ingroup qtopiaemb
*/

/*!
  \fn bool QUniqueId::operator==(const QUniqueId &o) const

  Returns true if the identifier is equal to \a o.  Otherwise returns false.
*/

/*!
  \fn bool QUniqueId::operator!=(const QUniqueId &o) const

  Returns true if the identifier is not equal to \a o.  Otherwise returns false.
*/

/*!
  \fn bool QUniqueId::operator<(QUniqueId o) const

  Returns true if the identifier is less than \a o.  Otherwise returns false.
*/

/*!
  \fn bool QUniqueId::operator>(QUniqueId o) const

  Returns true if the identifier is greater than \a o.  Otherwise returns false.
*/

/*!
  \fn bool QUniqueId::operator<=(QUniqueId o) const

  Returns true if the identifier is less than or equal to \a o.  Otherwise returns false.
*/

/*!
  \fn bool QUniqueId::operator>=(QUniqueId o) const

  Returns true if the identifier is greater than or equal to \a o.  Otherwise returns false.
*/

/*!
  \fn bool QUniqueId::isNull() const

  Returns true if the identifer is null.  Otherwise returns false.
*/

/*!
  Constructs A QUniqueId based of the string \a s.
*/
QUniqueId::QUniqueId(const QString &s)
{
    int pos = s.indexOf(':');
    if (pos < 0) {
        // then is short version
        pos = s.indexOf('-');
        mContext = s.left(pos).toInt();
        mId = s.mid(pos+1).toInt();
    } else {
        // long version
        QString left = s.left(pos);
        QString right = s.mid(pos+1);
        pos = right.indexOf(':');
        QString index = right.mid(pos+1);
        right = right.left(pos);
        mContext = uniqueIdTracker().reserveContext(left, right);

        if (pos > 0 && mContext) {
            mId = index.toInt();
        } else {
            mId = 0;
        }
    }
}

/*!
  Constructs a null QUniqueId.
*/
QUniqueId::QUniqueId() : mContext(0), mId(0) {}

/*!
  Constructs a deep copy of \a o.
*/
QUniqueId::QUniqueId(const QUniqueId &o) : mContext(o.mContext), mId(o.mId) {}

/*!
  Constructs A QUniqueId based of the byte array \a array.
*/
QUniqueId::QUniqueId(const QByteArray &array)
{
#ifndef QT_NO_DATASTREAM
    QDataStream ds(array);
    if (array.size() == 8) {
        // small version
        ds >> mContext;
        ds >> mId;
    } else {
        ds >> *this;
        // large version.
        if (!mContext)
            mId = 0;
    }
#endif
}

/*!
  Assigns the value of identifer \a o to this identifier.
*/
QUniqueId QUniqueId::operator=(const QUniqueId &o)
{
    mId = o.mId;
    mContext = o.mContext;
    return *this;
}

/*!
  Returns the index componenent of the identifer.
  \sa context(), device()
*/
uint QUniqueId::index() const
{
    return mId;
}

/*!
  Returns the context componenent of the identifer.
  \sa index(), device()
*/
QUuid QUniqueId::context() const
{
    return uniqueIdTracker().right(mContext);
}

/*!
  Returns the device componenent of the identifer.
  \sa index(), context()
*/
QUuid QUniqueId::device() const
{
    return uniqueIdTracker().left(mContext);
}

/*!
   Only for legacy use.
   Constructs an unscoped, local context id with from \a index. The identifier constructed will be
   equivalent to identifiers created by this function on other devices.
*/
QUniqueId::QUniqueId(uint index)
{
    if(index == 0)
    {
        QUniqueId();
        return;
    }
    QUniqueIdGenerator g(legacyIdContext(), legacyIdContext());
    QUniqueId o = g.constructUniqueId(index);
    mId = o.mId;
    mContext = o.mContext;
}

/*!
  Returns the id formatted as a string.  This string will only include \i latin1 characters and
  is suitable for passing between devices.
*/
QString QUniqueId::toString() const
{

    QUuid left = uniqueIdTracker().left(mContext);
    QUuid right = uniqueIdTracker().right(mContext);

    return left.toString() + ":" + right.toString() + ":" + QString::number(mId);
}

/*!
  Returns the id formatted as byte array.  This array is suitable for passing between devices.
*/
QByteArray QUniqueId::toByteArray() const
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds << *this;
    return data;
}

/*!
  Returns the id formatted as a string.  This string will only include \i latin1 characters,
  and is not suitable for passing between devices or writing to files that may be transfered.
  The string will be shorter than that provided by toString and is suitable to save space when
  storing the id locally.
*/
QString QUniqueId::toLocalContextString() const
{
    return QString::number(mContext) + "-" + QString::number(mId);
}

/*!
  Returns the id formatted as a byte array.
  The array is not suitable for passing between devices or writing to files
  that may be transfered.  The array will be shorter than that provided by toByteArray
  and is suitable to save space when storing the id locally.
*/
QByteArray QUniqueId::toLocalContextByteArray() const
{
#ifndef QT_NO_DATASTREAM
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    toLocalContextDataStream(s);
    return data;
#else
    return QByteArray();
#endif
}

#ifndef QT_NO_DATASTREAM
/*!
  Streams the id from the data stream \a s.  This function is not suitable for streaming data
  from another device or from a file that may have been transfered.
*/
QDataStream &QUniqueId::fromLocalContextDataStream( QDataStream &s )
{
    quint32 a;
    quint32 b;
    s >> a;
    s >> b;
    mContext = a;
    mId = b;
    return s;
}

/*!
  Streams the id to the data stream \a s.  This function is not suitable for streaming data
  to another device or to a file that may be transfered.
*/
QDataStream &QUniqueId::toLocalContextDataStream( QDataStream &s ) const
{
    s << (quint32) mContext;
    s << (quint32) mId;
    return s;
}
#endif

/*! \fn uint qHash(const QUniqueId &uid);

  Returns the hash value for \a uid.
 */

/*!
  \class QLocalUniqueId
  \brief The QLocalUniqueId class provides a convience class for storing QUniqueId objects locally
  on a device with reduced processing and space usage.

  It should not be used for when writing or reading id to another device or file that may
  be transfered.

  \sa QUniqueId, QUniqueIdGenerator

  \ingroup qtopiaemb
*/

/*!
  \fn QLocalUniqueId::QLocalUniqueId()

  \overload

  Constructs a null QLocalUniqueId.
*/

/*!
  \fn QLocalUniqueId::QLocalUniqueId(const QUniqueId &other)

  \overload

  Construct QLocalUniqueId as a copy of \a other.
*/

/*!
  \fn QLocalUniqueId::QLocalUniqueId(const QLocalUniqueId &other)
  \overload
  Construct a QLocalUniqueId as a copy of \a other.
*/

/*!
  \fn QLocalUniqueId::QLocalUniqueId(const QString &s)
  \overload

  Construcst a QLocalUniqueId based of the string \a s.
*/

/*!
  \fn QLocalUniqueId::QLocalUniqueId(const QByteArray &array)
  \overload
  Constructs A QLocalUniqueId based of the byte array \a array.
*/

/*!
  \fn QString QLocalUniqueId::toString() const

  Returns the id formatted as a string.  This string will only include latin1 characters.
  It is not suitable for passing between devices or writing to files that may be transfered.
*/

/*!
  \fn QByteArray QLocalUniqueId::toByteArray() const

  Returns the id formatted as a byte array.
  The array is not suitable for passing between devices or writing to files
  that may be transfered.
*/

/*!
  \fn QDataStream &QLocalUniqueId::fromLocalContextDataStream( QDataStream &s )

  Streams the id from the data stream \a s.  This function is not suitable for streaming data
  from another device or from a file that may have been transfered.
*/

/*!
  \fn QDataStream &QLocalUniqueId::toLocalContextDataStream( QDataStream &s ) const

  Streams the id to the data stream \a s.  This function is not suitable for streaming data
  to another device or to a file that may be transfered.
*/

Q_IMPLEMENT_USER_METATYPE(QUniqueId)
