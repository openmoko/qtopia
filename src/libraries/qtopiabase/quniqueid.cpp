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

    uint context(const QUuid &right) const;

    QUuid right(uint context) const;

    // returns next index starting at reserve spot.
    uint reserveIndex(uint context, uint count);
    uint reserveContext(const QUuid &right);

private:
    struct IdState {
        IdState() : nextId(1) {}
        IdState(const IdState &other)
            : right(other.right), nextId(other.nextId) {}
        QUuid right;
        uint nextId;
    };

    IdState *getItem(uint) const;
    uint getItem(const QUuid &right) const;

    bool compare(const IdState *, const QUuid &right);

    void read(IdState *item, QIODevice &dev) const
    {
        QUuid id;
        QDataStream ds(&dev);
        // read id to keep compat with previous tracker file versions.
        ds >> id;
        ds >> item->right;
        ds >> item->nextId;
    }
    void write(const IdState *item, QIODevice &dev) const
    {
        QDataStream ds(&dev);
        // write device id to keep compat with previous tracker file versions.
        ds << device;
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


    static const QUuid device;
};

// kept to maintain compatibility with previous tracker file versions..
const QUuid QUniqueIdTracker::device("802be8cf-25d6-43eb-88ec-1b2204ec668c");

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

static const uint tracker_version = 1;

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
        if (version != tracker_version) // current version
            qFatal("Invalid version id tracker file.");
        if (nextcontext > 0x000000ff)
            qFatal("Ids full"); // this device must have got a lot of use.
        // want a non-locking hold open.  memory file with changing size?
        file.close();
    } else {
        // init state
        file.open(QIODevice::WriteOnly);
        file.write(mkey, 16);
        write(tracker_version, file);
        write(1, file);
        // first context at 36 offset.  Mostly to allow context*context_size for seek.
        QByteArray nullData(12, 0);
        file.write(nullData);
        file.close();
    }
}

uint QUniqueIdTracker::context(const QUuid &right) const
{
    return getItem(right);
}

QUuid QUniqueIdTracker::right(uint context) const
{
    IdState *item = getItem(context);
    if (!item)
        return QUuid();
    return item->right;
}

static const uint context_size = 36;
static const uint next_context_offset = 20;
static const uint tracker_header_size = 36;

static uint contextPos(uint context) { return tracker_header_size + (context-1)*context_size; }

uint QUniqueIdTracker::reserveIndex(uint context, uint count)
{
    IdState *item = getItem(context); // also checks validity of context.
    if (!item)
        return 0; // no context exists.
    // TODO ipc and locking of file.
    file.open(QIODevice::ReadWrite);
    file.seek(contextPos(context));

    read(item, file); // refresh item.
    // should do an overflow check.
    item->nextId += count;
    file.seek(contextPos(context));
    write(item, file);

    file.close();
    return item->nextId - count;
}

uint QUniqueIdTracker::reserveContext(const QUuid &right)
{
    if (right.isNull())
        return 0;

    uint context = getItem(right);
    if (context)
        return context;
    // create a context.
    file.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    file.seek(next_context_offset);
    quint32 nextcontext;
    read(nextcontext, file);

    IdState *item = new IdState();
    item->right = right;
    item->nextId = 1;
    cache.insert(nextcontext, item);

    if(context_size*nextcontext > file.size()) {
        file.close();
        file.open(QIODevice::Append | QIODevice::Unbuffered);
    }
    else
        file.seek(contextPos(nextcontext));

    write(item, file);

    file.seek(next_context_offset);
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
    file.seek(next_context_offset);
    quint32 nextcontext;
    read(nextcontext, file);
    if (context >= nextcontext)
        return 0;
    file.seek(contextPos(context));
    IdState *item = new IdState();
    read(item, file);
    file.close();
    cache.insert(context, item);
    return item;
}

uint QUniqueIdTracker::getItem(const QUuid &right) const
{
    // this one much harder (and is reason for cache)
    QList<uint> keys = cache.keys();
    // first try in the cache.
    foreach(uint k, keys) {
        if (cache[k]->right == right)
            return k;
    }
    // not in cache.  now need to do file.  only cache result.
    file.open(QIODevice::ReadOnly);
    quint32 max;
    file.seek(next_context_offset);
    read(max, file);
    for (uint i = 1; i < max; i++) {
        file.seek(contextPos(i));
        IdState state;
        read(&state, file);
        if (right == state.right) {
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
    QUuid right = uniqueIdTracker().right(mappedContext());
    stream << right;
    stream << index();
}

/*!
    \fn void QUniqueId::deserialize(Stream &value)

    \internal

    Deserializes the QUniqueId instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QUniqueId::deserialize(Stream &stream)
{
    QUuid right;
    uint id;
    stream >> right;
    stream >> id;
    uint context = uniqueIdTracker().reserveContext(right);
    setIdentity(context, id);
}


QUuid QUniqueId::legacyIdContext()
{
    return QUuid("35a20342-a280-40d5-a18f-f45cd9d92e7d");
}

QUuid QUniqueId::temporaryIDContext()
{
    return QUuid("f3ed326a-f37a-4aa0-bad3-f7968e9ecd60");
}

/*!
  \class QUniqueIdGenerator
  \brief The QUniqueIdGenerator class provides a generator of unique identifiers.

  The QUniqueIdGenerator class provides a generator of unique identifiers.  They are
  32 bits, 4 bits of which make up a context and the remaining 24 bits an index component.
  This allows separate contexts to generated ids to be stored in a single list.  Unlike
  QUUid, QUniqueId is only unique upon a single device.

  Ids are generated with a QUuid Context identifier and an index.
  The context identifier should be generated with a program such as \a quuid and represent
  the data store or context within which the id is generated.  For example, XML, SQL , and SIM
  card storage for QContacts each have their own Context Identifier.  The Context is mapped
  to a 4bit value allowing for 255 contexts for a single device.

  The usual approach to constructing QUniqueIds is:

\code
  QUniqueIdGenerator g(myContext());
  id = g.createId();
\endcode

  QUniqueIdGenerator is suitable in cases where you need to generate an average of 100 identifiers
  a day or less and require a low number of bits when stored.  If your program is likely to need
  more than 100 unique identifiers a day or does not benefit from the low 32bit storage size it is
  recommended you look at another type of unique id, such as QUuid.

  \ingroup misc
*/

/*!
  \fn bool QUniqueIdGenerator::isValid() const

  Returns true if the generator is valid and can be used to create or construct
  unique ids.  Otherwise returns false.

  This function should only be needed during debugging as only a generator
  constructed using null QUuids will be invalid.
*/

/*!
  Create a QUniqueIdGenerator with the scope \a context.
  It is recommended that the context be hard coded into your application.  Generating
  \a context at run time is redundant.
*/
QUniqueIdGenerator::QUniqueIdGenerator(const QUuid &context)
    : mLastId(0), mReserved(0)
{
    mContext = uniqueIdTracker().reserveContext(context);
}

/*!
  Create a deep copy from \a other.
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
   Returns a locally mapped int for \a context, suitable for
   checking if a QUniqueId was generated from an equivalent
   QUniqueIdGenerator.

   \sa QUniqueId::mappedContext()
*/
uint QUniqueIdGenerator::mappedContext(const QUuid &context)
{
    return uniqueIdTracker().reserveContext(context);
}

/*!
  Creates a temporary identity based on \a index.

  Temporary identifiers are scoped so as not to conflict
  with other identifiers.
*/
QUniqueId QUniqueIdGenerator::temporaryID(uint index)
{
    QUniqueId i;
    uint context = uniqueIdTracker().reserveContext(QUniqueId::temporaryIDContext());
    i.setIdentity(context, index);
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
    uint context = mContext;
    uint id = mLastId;
    i.setIdentity(context, id);
    mReserved--;
    mLastId++;

    qLog(Support) << "createUniqueId" << i.toString();
    return i;
}

/*!
  Constructs a identifier based on \a index for the context of the generator.
  Does not ensure the constructed id is not in conflict with past or future QUniqueId objects.

  \sa createUniqueId()
*/
QUniqueId QUniqueIdGenerator::constructUniqueId(uint index)
{
    if (mContext == 0)
        return QUniqueId();

    QUniqueId i;
    uint context = mContext;
    uint id = index;
    i.setIdentity(context, id);
    return i;
}


/*!
  Returns true if this is a temporary identifier, otherwise returns false.

  Temporary identifiers are scoped so as not to conflict with other identifiers.

  They are suitable for working with data sets when the ids are not
  intended to be permanent.
*/
bool QUniqueId::isTemporary() const
{
    // faster to look up via context.
    QUuid c = uniqueIdTracker().right(mappedContext());
    return c == temporaryIDContext();
}

/*!
  \class QUniqueId
  \brief The QUniqueId class provides an identifier that is unique and yet can be stored
  efficiently.

  The QUniqueId class provides a 32 bit identifier that is unique to the device it is created
  on.  The are composed of an 8 bit context representing the data source that created them
  and a 24 bit index component.  They are intended for the case of record identifiers for
  data such as contacts, however given the limited space of identifiers they are unsuitable
  for use as identifiers that might get created on the order of thousands per day.

  The context identifier should be generated with a program such as \a quuid and represent
  the data store or context within which the id is generated.  For example, XML, SQL, and SIM
  card storage for QContacts each have their own Context Identifier.

  \sa QUniqueIdGenerator

  \ingroup misc
*/

/*!
   Returns a locally mapped int for the context of the id.
   This is useful checking if a QUniqueId was generated from an equivalent
   QUniqueIdGenerator.

   \sa QUniqueIdGenerator::mappedContext()
*/
uint QUniqueId::mappedContext() const
{
    uint context = mId >> 24;
    return context;
}

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

  Returns true if the identifier is null.  Otherwise returns false.
*/

/*!
  Constructs A QUniqueId based on the string \a s.
*/
QUniqueId::QUniqueId(const QString &s)
{
    int pos = s.indexOf(':');
    if (pos < 0) {
        // then is short version
        pos = s.indexOf('-');
        uint context = s.left(pos).toInt();
        uint id = s.mid(pos+1).toInt();
        setIdentity(context, id);
    } else {
        // long version
        // reads both uid:uid:index and uid:index, the later containing no device id.

        QStringList sl = s.split(':');

        if (sl.count() == 3) {
            // Old format, ignore the first uid (device id)
            sl.removeFirst();
        }

        // Now we should only have two parts
        if (sl.count() == 2) {
            uint context = uniqueIdTracker().reserveContext(sl[0]);
            setIdentity(context, sl[1].toInt());
        } else {
            setIdentity(0,0);
        }
    }
}

/*!
  Constructs a null QUniqueId.
*/
QUniqueId::QUniqueId() : mId(0) {}

/*!
  Constructs a deep copy of \a o.
*/
QUniqueId::QUniqueId(const QUniqueId &o) : mId(o.mId) {}

/*!
  Constructs A QUniqueId based on the byte array \a array.
*/
QUniqueId::QUniqueId(const QByteArray &array)
{
#ifndef QT_NO_DATASTREAM
    if (array.size() == 8) {
        memcpy(&mId, array.constData(), 4);
    } else {
        QDataStream ds(array);
        if (array.size() == 36) {
            QUuid legacyid; // thrown away, no longer used.
            ds >> legacyid;
        }
        ds >> *this;
    }
#endif
}

/*!
  Assigns the value of identifier \a o to this identifier.
*/
QUniqueId QUniqueId::operator=(const QUniqueId &o)
{
    mId = o.mId;
    return *this;
}

/*!
  Returns the index component of the identifier.
  \sa context()
*/
uint QUniqueId::index() const
{
    return 0x00ffffff & mId;
}

/*!
  Sets the context component of the id to \a context.
 */
void QUniqueId::setContext(uint context)
{
    mId &= 0x00ffffff; // clear old context;
    mId |= (context << 24); // add new context
}

/*!
  Sets the index component of the id to \a index.
 */
void QUniqueId::setIndex(uint index)
{
    mId &= 0xff000000; // clear old index;
    mId |= (0x00ffffff & index);
}

void QUniqueId::setIdentity(uint context, uint index)
{
    if (context == 0)
        mId = 0;
    else
        mId = (context << 24) | (0x00ffffff & index);
}

/*!
  Returns the context component of the identifier.
  \sa index()
*/
QUuid QUniqueId::context() const
{
    return uniqueIdTracker().right(mappedContext());
}

/*!
  Returns the id formatted as a string.  This string will only include \i latin1 characters and
  contains the expanded form of the context.
  This allows the id to be backed up independent
  of the internal mapping of context QUuid to the internal context component.
*/
QString QUniqueId::toString() const
{

    QUuid right = uniqueIdTracker().right(mappedContext());

    return right.toString() + ":" + QString::number(index());
}

/*!
  Returns the id formatted as a byte array.  The byte array contains the expanded form of the
  context.  This allows the id to be backed up independent
  of the internal mapping of context QUuid to the internal context component.
*/
QByteArray QUniqueId::toByteArray() const
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds << *this;
    return data;
}

/*!
  Returns the id packed as a 32 bit uint.  This contains both the context and the index components of the id.
*/
uint QUniqueId::toUInt() const
{
    return mId;
}

/*!
  Returns the QUniqueId represented by the packed value \a identifier.
*/
QUniqueId QUniqueId::fromUInt(uint identifier)
{
    QUniqueId id;
    id.mId = identifier;
    return id;
}

/*!
  Returns the id formatted as a string.  This string will only include \i latin1 characters,
  and will contains the mapped context.  If the tracking or generating file is reset this
  string won't be usable to gain the original id.

  \sa toString()
*/
QString QUniqueId::toLocalContextString() const
{
    return QString::number(mappedContext()) + "-" + QString::number(index());
}

/*!
  Returns a QUniqueId that is valid only within a single application.  This
  QUniqueId should not be shared or persisted between different applications or
  even different executions of the same application.

  This function is not threadsafe.
 */
QUniqueId QUniqueId::constructApplicationLocalUniqueId()
{
    static unsigned int nextId = 0;
    QUniqueId id;
    id.setIdentity(0, ++nextId);
    return id;
}

/*!
  Returns the id formatted as a byte array.  The mapped representation of the context
  will be written rather than the expanded QUuid.  If the tracking or generating file is
  reset this byte array won't be usable to gain the original id.

  \sa toByteArray()
*/
QByteArray QUniqueId::toLocalContextByteArray() const
{
    QByteArray data((const char *)&mId, 8);
    return data;
}

#ifndef QT_NO_DATASTREAM
/*!
  Streams the id from the data stream \a s.  This function is not suitable for streaming data
  from another device or from a file that may have been transferred.
*/
QDataStream &QUniqueId::fromLocalContextDataStream( QDataStream &s )
{
    quint32 a;
    s >> a;
    mId = a;
    return s;
}

/*!
  Streams the id to the data stream \a s.  This function is not suitable for streaming data
  to another device or to a file that may be transferred.
*/
QDataStream &QUniqueId::toLocalContextDataStream( QDataStream &s ) const
{
    s << (quint32) mId;
    return s;
}
#endif

/*! \fn uint qHash(const QUniqueId &uid);

  Returns the hash value for \a uid.
 */

Q_IMPLEMENT_USER_METATYPE(QUniqueId)
