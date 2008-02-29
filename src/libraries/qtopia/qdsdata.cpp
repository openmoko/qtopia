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

// Local includes
#include "qdsdata.h"
#include "qdsdata_p.h"

// Qt includes
#include <QFile>
#include <QTemporaryFile>
#include <QBuffer>

// Qtopia includes
#include <QMimeType>
#include <qtopialog.h>
#include <QStorageMetaInfo>

// Macros
Q_IMPLEMENT_USER_METATYPE(QDSData);

// ============================================================================
//
//  Constants
//
// ============================================================================

static const int        QDS_DATA_SIZE_DEFAULT_THRESHOLD =   128;

static const QString    QDSDATASTORE_PATH   = "QDSDataStore";
static const QString    INFO_FILE_EXTENSION = ".info";
static const QString    DATA_FILE_EXTENSION = ".data";

// ============================================================================
//
//  QDSDataStore
//
// ============================================================================

bool QDSDataStore::add( const QLocalUniqueId& id,
                        const QByteArray& data,
                        const QMimeType& type )
{
    // Create info and data files (in an atomic fashion)
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( infoFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::add - info file already exists";
        return false;
    }

    if ( !infoFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::add - "
                          << "info file can't be written to";
        return false;
    }

    QDSLockedFile dataFile( dataFileName( id ) );
    if ( dataFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::add - data file already exists";
        return false;
    }

    if ( !dataFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::add - "
                          << "data file can't be written to";
        return false;
    }

    // Write to files
    QDataStream infoDs( &infoFile );
    infoDs << 1;
    infoDs << 0;
    infoDs << type.id();

    dataFile.write( data );

    return true;
}

bool QDSDataStore::add( const QLocalUniqueId& id,
                        QFile& data,
                        const QMimeType& type )
{
    // Create info file and copy data file (in an atomic fashion)
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( infoFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::add - info file already exists";
        return false;
    }

    if ( !infoFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::add - "
                          << "info file can't be written to";
        return false;
    }

    if ( !data.copy( dataFileName( id ) ) ) {
        qLog(DataSharing) << "QDSDataStore::add - couldn't copy data file";
        return false;
    }

    QDataStream ds(&infoFile);
    ds << 1;
    ds << 0;
    ds << type.id();

    return true;
}

bool QDSDataStore::add( const QLocalUniqueId& id )
{
    // Update info file (in an atomic fashion)
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( !infoFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::add - info file doesn't exist";
        return false;
    }

    int refCount = 0;
    int transCount = 0;
    QString type;
    if ( infoFile.open( QIODevice::ReadWrite ) ) {
        QDataStream ds(&infoFile);
        ds >> refCount;
        ds >> transCount;
        ds >> type;
        infoFile.seek( 0 );
        ds << refCount + 1;
        ds << transCount;
        ds << type;
        infoFile.close();
    } else {
        qLog(DataSharing) << "QDSDataStore::add - info file can't be opened";
        return false;
    }

    return true;
}

bool QDSDataStore::set( const QLocalUniqueId& id, const QByteArray& data )
{
    // Open data file
    QDSLockedFile dataFile( dataFileName( id ) );
    if ( !dataFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::set - data file doesn't exist";
        return false;
    }

    if ( !dataFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::set - "
                          << "data file can't be written to";
        return false;
    }

    // Overwrite old data file
    dataFile.write( data );

    return true;
}


bool QDSDataStore::set( const QLocalUniqueId& id, QFile& data )
{
    // Overwrite old data file with new data file
    QDSLockedFile dataFile( dataFileName( id ) );
    if ( !dataFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::set - "
                          << "data file can't be written to";
        return false;
    }

    bool closeAfter = false;
    if ( !data.isOpen() ) {
        if ( !data.open( QIODevice::ReadOnly ) ) {
            qLog(DataSharing) << "QDSDataStore::set - "
                              << "new data can't be read";
            return false;
        }
        closeAfter = true;
    }

    dataFile.write( data.readAll() );

    if ( closeAfter )
        data.close();

    return true;
}

bool QDSDataStore::set( const QLocalUniqueId& id,
                        const QByteArray& data,
                        const QMimeType& type )
{
    // Get current reference count
    int refCount = referenceCount( id );
    int transCount = transmitCount( id );

    // Open files (do this in an atomic fashion)
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( !infoFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::set - info file doesn't exist";
        return false;
    }

    if ( !infoFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::set - "
                          << "info file can't be written to";
        return false;
    }

    QDSLockedFile dataFile( dataFileName( id ) );
    if ( !dataFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::set - data file doesn't exist";
        return false;
    }

    if ( !dataFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::set - "
                          << "data file can't be written to";
        return false;
    }

    // Overwrite old info file
    QDataStream infoDs(&infoFile);
    infoDs << refCount;
    infoDs << transCount;
    infoDs << type.id();

    // Overwrite old data file
    dataFile.write( data );

    return true;
}


bool QDSDataStore::set( const QLocalUniqueId& id,
                        QFile& data,
                        const QMimeType& type )
{
    // Get old reference count
    int refCount = referenceCount( id );
    int transCount = transmitCount( id );

    // Open info file and remove old data file (in an atomic fashion)
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( !infoFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::set - info file doesn't exist";
        return false;
    }

    if ( !infoFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::set - "
                          << "info file can't be written to";
        return false;
    }

    // Overwrite old data file with new data file
    QDSLockedFile dataFile( dataFileName( id ) );
    if ( !dataFile.open( QIODevice::WriteOnly ) ) {
        qLog(DataSharing) << "QDSDataStore::set - "
                          << "data file can't be written to";
        return false;
    }

    bool closeAfter = false;
    if ( !data.isOpen() ) {
        if ( !data.open( QIODevice::ReadOnly ) ) {
            qLog(DataSharing) << "QDSDataStore::set - "
                              << "new data can't be read";
            return false;
        }
    }

    dataFile.write( data.readAll() );

    if ( closeAfter )
        data.close();

    {
        QDataStream ds(&infoFile);
        ds << refCount;
        ds << transCount;
        ds << type.id();
    }

    return true;
}

bool QDSDataStore::remove( const QLocalUniqueId &id )
{
    if ( ( referenceCount( id ) <= 1 ) && ( transmitCount( id ) <= 0 ) ) {
        // Try removing the data file first as it is the most likely to be open
        // for reading due to a QDSData::toIODevice() call
        if ( !QFile::remove( dataFileName( id ) ) ) {
            qLog(DataSharing) << "QDSDataStore::remove - "
                              << "couldn't remove old data file";
            return false;
        }

        if ( !QFile::remove( infoFileName( id ) ) ) {
            qLog(DataSharing) << "QDSDataStore::remove - "
                              << "couldn't remove old info file";
            return false;
        }
    } else {
        removeReference( id );
    }

    return true;
}

bool QDSDataStore::exists( const QLocalUniqueId &id )
{
    QDSLockedFile infoFile( infoFileName( id ) );
    QDSLockedFile dataFile( dataFileName( id ) );

    return infoFile.exists() && dataFile.exists();
}

bool QDSDataStore::transmit( const QLocalUniqueId& id )
{
    // Update info file (in an atomic fashion)
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( !infoFile.exists() ) {
        qLog(DataSharing) << "QDSDataStore::transmit - info file doesn't exist";
        return false;
    }

    int refCount = 0;
    int transCount = 0;
    QString type;
    if ( infoFile.open( QIODevice::ReadWrite ) ) {
        QDataStream ds(&infoFile);
        ds >> refCount;
        ds >> transCount;
        ds >> type;
        infoFile.seek( 0 );
        ds << refCount;
        ds << transCount + 1;
        ds << type;
        infoFile.close();
    } else {
        qLog(DataSharing) << "QDSDataStore::transmit - info file can't be opened";
        return false;
    }

    return true;
}

bool QDSDataStore::received( const QLocalUniqueId& id )
{
    if ( ( referenceCount( id ) <= 0 ) && ( transmitCount( id ) <= 1 ) ) {
        // Try removing the data file first as it is the most likely to be open
        // for reading due to a QDSData::toIODevice() call
        if ( !QFile::remove( dataFileName( id ) ) ) {
            qLog(DataSharing) << "QDSDataStore::remove - "
                              << "couldn't remove old data file";
            return false;
        }

        if ( !QFile::remove( infoFileName( id ) ) ) {
            qLog(DataSharing) << "QDSDataStore::remove - "
                              << "couldn't remove old info file";
            return false;
        }
    } else {
        removeTransmit( id );
    }

    return true;
}

QMimeType QDSDataStore::type( const QLocalUniqueId &id )
{
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( infoFile.exists() && infoFile.open( QIODevice::ReadOnly ) ) {
        QDataStream ds( &infoFile );
        QString type;
        int refCount;
        int transCount;
        ds >> refCount;
        ds >> transCount;
        ds >> type;
        return QMimeType(type);
    } else {
        qLog(DataSharing) << "QDSDataStore::type - couldn't access info file";
        return QMimeType( QString() );
    }
}

QByteArray QDSDataStore::data( const QLocalUniqueId &id )
{
    QDSLockedFile dataFile( dataFileName( id ) );
    if ( dataFile.exists() && dataFile.open( QIODevice::ReadOnly ) ) {
        return dataFile.readAll();
    } else {
        qLog(DataSharing) << "QDSDataStore::data - couldn't access data file";
        return QByteArray();
    }
}

QString QDSDataStore::infoFileName( const QLocalUniqueId& id )
{
    return Qtopia::applicationFileName( QDSDATASTORE_PATH,
                                        id.toString() + INFO_FILE_EXTENSION );
}

QString QDSDataStore::dataFileName( const QLocalUniqueId& id )
{
    return Qtopia::applicationFileName( QDSDATASTORE_PATH,
                                        id.toString() + DATA_FILE_EXTENSION );
}


int QDSDataStore::referenceCount( const QLocalUniqueId &id )
{
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( infoFile.exists() && infoFile.open( QIODevice::ReadOnly ) ) {
        QDataStream ds( &infoFile );
        int refCount = 0;
        ds >> refCount;
        return refCount;
    } else {
        qLog(DataSharing) << "QDSDataStore::referenceCount - "
                          << "couldn't access info file";
        return -1;
    }
}

bool QDSDataStore::removeReference( const QLocalUniqueId& id )
{
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( infoFile.exists() && infoFile.open( QIODevice::ReadWrite ) ) {
        QDataStream ds(&infoFile);

        // Get the current reference count and mime type
        int refCount;
        int transCount;
        QString type;
        ds >> refCount;
        ds >> transCount;
        ds >> type;

        // Decrement the reference count and write to the info file
        if ( refCount <= 0 )
            return false;
        --refCount;
        infoFile.seek( 0 );
        ds << refCount;
        ds << transCount;
        ds << type;
        infoFile.close();
    } else {
        qLog(DataSharing) << "QDSDataStore::removeReference - "
                          << "couldn't write to info file";
        return false;
    }

    return true;
}

int QDSDataStore::transmitCount( const QLocalUniqueId& id )
{
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( infoFile.exists() && infoFile.open( QIODevice::ReadOnly ) ) {
        QDataStream ds( &infoFile );
        int refCount;
        int transCount;
        ds >> refCount;
        ds >> transCount;
        return transCount;
    } else {
        qLog(DataSharing) << "QDSDataStore::transmitCount - "
                          << "couldn't access info file";
        return -1;
    }
}

bool QDSDataStore::removeTransmit( const QLocalUniqueId& id )
{
    QDSLockedFile infoFile( infoFileName( id ) );
    if ( infoFile.exists() && infoFile.open( QIODevice::ReadWrite ) ) {
        QDataStream ds(&infoFile);

        // Get the current transmit count and mime type
        int refCount;
        int transCount;
        QString type;
        ds >> refCount;
        ds >> transCount;
        ds >> type;

        // Decrement the transimission count and write to the info file
        if ( transCount <= 0 )
            return false;
        --transCount;
        infoFile.seek( 0 );
        ds << refCount;
        ds << transCount;
        ds << type;
        infoFile.close();
    } else {
        qLog(DataSharing) << "QDSDataStore::removeTransmit - "
                          << "couldn't write to info file";
        return false;
    }

    return true;
}

// ============================================================================
//
//  QDSDataThreshold
//
// ============================================================================

QDSDataThreshold::QDSDataThreshold()
:   mValue( QDS_DATA_SIZE_DEFAULT_THRESHOLD )
{
    QStorageMetaInfo storage;
    storage.update();
    const QFileSystem* qdsMount
        = storage.fileSystemOf(
            Qtopia::applicationFileName( QDSDATASTORE_PATH, QString() ) );
    if ( qdsMount == 0 ) {
        qLog(DataSharing) << "No mount for QDS";
        mValue = -1;
    } else {
        if ( !qdsMount->isWritable() ) {
            qLog(DataSharing) << "QDS mount not writable!";
            mValue = -1;
        }
    }
}

// ============================================================================
//
//  QDSLockedFile
//
// ============================================================================

// QDSLockedFile is not currently being used as this class doesn't work on
// the greenphone

QDSLockedFile::QDSLockedFile()
:   QFile(),
    mLock( NoLock )
{
}

QDSLockedFile::QDSLockedFile( const QString& name )
:   QFile( name ),
    mLock( NoLock )
{
}

QDSLockedFile::~QDSLockedFile()
{
    unlock();
}

bool QDSLockedFile::open( QIODevice::OpenMode mode, bool acquireLock )
{
    if ( !QFile::open( mode ) )
        return false;

    if ( !acquireLock )
        return true;

    if ( mode == QIODevice::ReadOnly )
        return lock( ReadLock );
    else
        return lock( WriteLock );
}

void QDSLockedFile::close()
{
    unlock();
    QFile::close();
}

bool QDSLockedFile::isLocked() const
{
    if ( ( mLock == ReadLock ) || ( mLock == WriteLock ) )
        return true;

    return false;
}

bool QDSLockedFile::lock( LockMode mode, bool block )
{
    if ( mode == NoLock )
        return false;

    if ( !isOpen() ) {
        qLog(DataSharing) << "QDSLockedFile::lock - "
                          << "file hasn't been opened";
        return false;
    }

    int operation = -1;
    if ( mode == ReadLock )
        operation = Qtopia::LockShare;
    else if ( mode == WriteLock )
        operation = Qtopia::LockWrite;

    if ( !block )
        operation |= Qtopia::LockBlock;

    if ( !Qtopia::lockFile( *this, operation ) )
        return false;

    mLock = mode;
    return true;
}

QDSLockedFile::LockMode QDSLockedFile::lockMode() const
{
    return mLock;
}

bool QDSLockedFile::unlock()
{
    if ( !Qtopia::unlockFile( *this ) )
        return false;

    mLock = NoLock;
    return true;
}

bool QDSLockedFile::remove()
{
    return QFile::remove();
}

bool QDSLockedFile::remove( const QString& name )
{
    QDSLockedFile remfile( name );
    if ( !remfile.lock( WriteLock ) )
        return false;

    return remfile.remove();
}

// ============================================================================
//
//  QDSDataPrivate
//
// ============================================================================

// Created with uuidgen
QUniqueIdGenerator QDSDataPrivate::mIdGen(
    "b9055662-d3bc-4c6d-8dea-f89bbb5f4752" ); // No tr
QDSDataThreshold   QDSDataPrivate::mThreshold;

QDSDataPrivate::QDSDataPrivate()
:   mUsingLocalStore( false ),
    mId(),
    mLocalStore( 0 ),
    mType( 0 )
{
}

QDSDataPrivate::QDSDataPrivate( const QLocalUniqueId& dataId )
:   mUsingLocalStore( false ),
    mId( dataId ),
    mLocalStore( 0 ),
    mType( 0 )
{
    if ( !QDSDataStore::add( mId ) )
        mId = QLocalUniqueId();
}

QDSDataPrivate::QDSDataPrivate( const QByteArray& data, const QMimeType& type )
:   mUsingLocalStore( false ),
    mId(),
    mLocalStore( 0 ),
    mType( 0 )
{
    mId = mIdGen.createUniqueId();

    if ( ( mThreshold.mValue == -1 ) ||
         ( data.size() < mThreshold.mValue ) ) {
        mUsingLocalStore = true;
        mLocalStore = new QByteArray( data );
        mType = new QMimeType( type );
    } else {
        if ( !QDSDataStore::add( mId, data, type ) )
            mId = QLocalUniqueId();
    }
}

QDSDataPrivate::QDSDataPrivate( QFile& data, const QMimeType& type )
:   mUsingLocalStore( false ),
    mId(),
    mLocalStore( 0 ),
    mType( 0 )
{
    mId = mIdGen.createUniqueId();

    if ( ( mThreshold.mValue == -1 ) ||
         ( data.size() < mThreshold.mValue ) ) {
        mUsingLocalStore = true;
        if ( !data.isOpen() ) {
            if ( !data.open( QIODevice::ReadOnly ) ) {
                qLog(DataSharing) << "QDSDataPrivate::QDSDataPrivate - "
                                  << "can't open file";
            }
        }
        mLocalStore = new QByteArray( data.readAll() );
        data.close();
        mType = new QMimeType( type );
    } else {
        if ( !QDSDataStore::add( mId, data, type ) )
            mId = QLocalUniqueId();
    }
}

QDSDataPrivate::QDSDataPrivate( const QLocalUniqueId& dataId,
                                const QByteArray& data,
                                const QMimeType& type )
:   mUsingLocalStore( false ),
    mId( dataId ),
    mLocalStore( 0 ),
    mType( 0 )
{
    if ( ( mThreshold.mValue == -1 ) ||
         ( data.size() < mThreshold.mValue ) ) {
        mUsingLocalStore = true;
        mLocalStore = new QByteArray( data );
        mType = new QMimeType( type );
    } else {
        if ( !QDSDataStore::add( mId, data, type ) )
            mId = QLocalUniqueId();
    }
}

QDSDataPrivate::~QDSDataPrivate()
{
    if ( !mUsingLocalStore && ( mId != QLocalUniqueId() ) )
        QDSDataStore::remove( mId );

    delete mLocalStore;
    mLocalStore = 0;

    delete mType;
    mType = 0;
}

void QDSDataPrivate::shiftToStore()
{
    if ( mThreshold.mValue == -1 ) {
        qLog(DataSharing) << "QDSDataPrivate::shiftToStore - "
                          << "can't write file on read-only filesystem";
        return;
    }

    if ( mUsingLocalStore ) {
        QDSDataStore::add( mId, *mLocalStore, *mType );
        delete mLocalStore;
        mLocalStore = 0;
        delete mType;
        mType = 0;
        mUsingLocalStore = false;
    }
}

// ============================================================================
//
//  QDSData
//
// ============================================================================

/*!
    \class QDSData
    \brief The QDSData class encapsulates data which is shared with QDS
    services.

    The QDSData class is used with QDS services as an efficient means of
    sharing data between client and provider applications. The data is
    implicitly shared between QDSData objects, and can be stored persistently.

    For larger data sizes methods which use QFile and QIODevice are recommended.

    \sa QDSAction, QDSActionRequest
*/

/*!
    The default constructor creates an empty object.
*/
QDSData::QDSData()
:   d( 0 )
{
    // Create d pointer
    d = new QDSDataPrivate();
}

/*!
    Copy constructor creates a deep copy of \a{other}.
*/
QDSData::QDSData( const QDSData& other )
:   d( 0 )
{
    // Create d pointer
    if ( other.d->mId != QLocalUniqueId() ) {
        if ( other.d != 0 ) {
            if ( other.d->mUsingLocalStore )
                other.d->shiftToStore();
            d = new QDSDataPrivate( other.d->mId );
        }
    } else {
        d = new QDSDataPrivate();
    }
}

/*!
    Constructs a data object from the \a key created by QDSData::store().
*/
QDSData::QDSData( const QLocalUniqueId& key )
:   d( 0 )
{
    // Construct the d pointer
    d = new QDSDataPrivate( key );
}

/*!
    Constructs a new data object with \a data of \a type.
*/
QDSData::QDSData( const QByteArray& data, const QMimeType& type )
:   d( 0 )
{
    d = new QDSDataPrivate( data, type );
}

/*!
    Constructs a new data object with \a data of \a type. \a data should not contain
    non-Latin1 characters as it is converted to a Latin1 representation.
*/
QDSData::QDSData( const QString& data, const QMimeType& type )
:   d( 0 )
{
    d = new QDSDataPrivate( data.toLatin1(), type );
}

/*!
    Constructs a new data object with \a data of \a type. The data in \a data
    is copied into the QDS data store.
*/
QDSData::QDSData( QFile& data, const QMimeType& type )
:   d( 0 )
{
    d = new QDSDataPrivate( data, type );
}

/*!
    Destructor.
*/
QDSData::~QDSData()
{
    delete d;
    d = 0;
}

/*!
    Assignment operator which deep copies \a other.
*/
const QDSData& QDSData::operator=( const QDSData& other )
{
    // Delete old d pointer
    delete d;
    d = 0;

    // Copy other d pointer
    if ( other.d->mId != QLocalUniqueId() ) {
        if ( other.d->mUsingLocalStore )
            other.d->shiftToStore();
        d = new QDSDataPrivate( other.d->mId );
    } else {
        d = new QDSDataPrivate();
    }

    return *this;
}

/*!
    Comparison operator which returns true if \a other is equivalent.
*/
bool QDSData::operator==( const QDSData& other ) const
{
    if ( ( d == 0 ) && ( other.d == 0 ) ) {
        return true;
    } else if ( ( d == 0 ) && ( other.d != 0 ) ) {
        return false;
    } else if ( ( d != 0 ) && ( other.d == 0 ) ) {
        return false;
    }

    if ( d->mId != other.d->mId )
        return false;

    if ( d->mUsingLocalStore != other.d->mUsingLocalStore )
        return false;

    if ( d->mUsingLocalStore ) {
        if ( *(d->mLocalStore) != *(other.d->mLocalStore) )
            return false;
        if ( d->mType->id() != other.d->mType->id() )
            return false;
    }

    return true;
}

/*!
    Comparison operator which returns true if \a other is not equivalent.
*/
bool QDSData::operator!=( const QDSData& other ) const
{
    return !operator==( other );
}

/*!
    Determines if the data object is valid.
*/
bool QDSData::isValid() const
{
    if ( d->mId != QLocalUniqueId() )
        return true;
    else
        return false;
}

/*!
    Returns the data type.
*/
QMimeType QDSData::type() const
{
    if ( d->mUsingLocalStore ) {
        return *(d->mType);
    } else if ( isValid() ) {
        return QDSDataStore::type( d->mId );
    }

    return QMimeType( QString() );
}

/*!
    Returns the data as a QByteArray.
*/
QByteArray QDSData::data() const
{
    if ( d->mUsingLocalStore ) {
        return *(d->mLocalStore);
    } else if ( isValid() ) {
        return QDSDataStore::data( d->mId );
    }

    return QByteArray();
}

/*!
    Returns the data as a QString.
*/
QString QDSData::toString() const
{
    if ( d->mUsingLocalStore ) {
        return QString( *(d->mLocalStore) );
    } else if ( isValid() ) {
        return QString( QDSDataStore::data( d->mId ) );
    }

    return QString();
}

/*!
    Provides direct read-only access to the data using the QIODevice interface. The
    caller owns the returned QIODevice pointer, and must delete it when finished.

    The pointer can be used to access the data using a QDataStream.
*/
QIODevice* QDSData::toIODevice() const
{
    // Test that you can cast this back to it's original type when returned

    if ( d->mUsingLocalStore ) {
        QBuffer* buffer = new QBuffer( d->mLocalStore );
        buffer->open( QIODevice::ReadOnly );
        return buffer;
    } else if ( isValid() ) {
        QFile* file = new QFile( QDSDataStore::dataFileName( d->mId ) );
        if ( file->exists() && file->open( QIODevice::ReadOnly ) ) {
            return file;
        } else {
            delete file;
            return 0;
        }
    }

    return 0;
}

/*!
    Adds the data to the persistent data store, and returns a key for accessing
    the data in the future.

    If the data was correctly stored, the returned key will not be null.

    The persistent data is reference counted so all applications who wish to
    access the data from the data store should call this method.
*/
QLocalUniqueId QDSData::store()
{
    if ( !isValid() )
        return QLocalUniqueId();

    // If we are using the local store, shift the data into the persistent store
    if ( d->mUsingLocalStore )
        d->shiftToStore();

    // Increment the reference count
    if ( !QDSDataStore::add( d->mId ) )
        return QLocalUniqueId();

    // Return the unique id as the key
    return d->mId;
}

/*!
    Removes the data from the persistent data store, when the reference count
    is zero, indicating that no-one requires the data, the data is deleted
    permanently from the data store.
*/
bool QDSData::remove()
{
    if ( !isValid() )
        return false;

    return QDSDataStore::remove( d->mId );
}

/*!
    Replaces the data with \a data. It is assumed that \a data is of the same
    data type as the original data.
*/
bool QDSData::modify( const QByteArray& data )
{
    bool ret = false;
    if ( d->mUsingLocalStore ) {
        delete d->mLocalStore;
        d->mLocalStore = 0;
        if ( ( d->mThreshold.mValue == -1 ) ||
             ( data.size() < d->mThreshold.mValue ) ) {
            d->mLocalStore = new QByteArray( data );
            ret = true;
        } else {
            d->mUsingLocalStore = false;
            ret = QDSDataStore::add( d->mId, data, *(d->mType) );
            delete d->mType;
            d->mType = 0;
        }
    } else if ( isValid() ) {
        ret = QDSDataStore::set( d->mId, data );
    }

    return ret;
}

/*!
    Replaces the data with \a data. It is assumed that \a data is of the same
    data type as the original data.
*/
bool QDSData::modify( const QString& data )
{
    bool ret = false;
    if ( d->mUsingLocalStore ) {
        delete d->mLocalStore;
        d->mLocalStore = 0;
        if ( ( d->mThreshold.mValue == -1 ) ||
             ( data.size() < d->mThreshold.mValue ) ) {
            d->mLocalStore = new QByteArray( data.toLatin1() );
            ret = true;
        } else {
            d->mUsingLocalStore = false;
            ret = QDSDataStore::add( d->mId, data.toLatin1(), *(d->mType) );
            delete d->mType;
            d->mType = 0;
        }
    } else if ( isValid() ) {
        ret = QDSDataStore::set( d->mId, data.toLatin1() );
    }

    return ret;
}

/*!
    Replaces the data with \a data. It is assumed that \a data is of the same
    data type as the original data.
*/
bool QDSData::modify( QFile& data )
{
    bool ret = false;
    if ( d->mUsingLocalStore ) {
        delete d->mLocalStore;
        d->mLocalStore = 0;

        if ( ( d->mThreshold.mValue == -1 ) ||
             ( data.size() < d->mThreshold.mValue ) ) {
            if ( !data.isOpen() ) {
                if ( !data.open( QIODevice::ReadOnly ) ) {
                    qLog(DataSharing) << "QDSDataPrivate::QDSDataPrivate - "
                                      << "can't open file";
                }
            }
            d->mLocalStore = new QByteArray( data.readAll() );
            data.close();
            ret = true;
        } else {
            d->mUsingLocalStore = false;
            ret = QDSDataStore::add( d->mId, data, *(d->mType) );
            delete d->mType;
            d->mType = 0;
        }
    } else if ( isValid() ) {
        ret = QDSDataStore::set( d->mId, data );
    }

    return ret;
}

/*!
    Replaces the data with \a data of \a type.
*/
bool QDSData::modify( const QByteArray& data, const QMimeType& type )
{
    bool ret = false;
    if ( d->mUsingLocalStore ) {
        delete d->mLocalStore;
        d->mLocalStore = 0;
        delete d->mType;
        d->mType = 0;
        if ( ( d->mThreshold.mValue == -1 ) ||
             ( data.size() < d->mThreshold.mValue ) ) {
            d->mLocalStore = new QByteArray( data );
            d->mType = new QMimeType( type );
            ret = true;
        } else {
            d->mUsingLocalStore = false;
            ret = QDSDataStore::add( d->mId, data, type );
        }
    } else if ( isValid() ) {
        ret = QDSDataStore::set( d->mId, data, type );
    }

    return ret;
}

/*!
    Replaces the data with \a data of \a type.
*/
bool QDSData::modify( const QString& data, const QMimeType& type )
{
    bool ret = false;
    if ( d->mUsingLocalStore ) {
        delete d->mLocalStore;
        d->mLocalStore = 0;
        delete d->mType;
        d->mType = 0;
        if ( ( d->mThreshold.mValue == -1 ) ||
             ( data.size() < d->mThreshold.mValue ) ) {
            d->mLocalStore = new QByteArray( data.toLatin1() );
            d->mType = new QMimeType( type );
            ret = true;
        } else {
            d->mUsingLocalStore = false;
            ret = QDSDataStore::add( d->mId, data.toLatin1(), type );
        }
    } else if ( isValid() ) {
        ret = QDSDataStore::set( d->mId, data.toLatin1(), type );
    }

    return ret;
}

/*!
    Replaces the data with \a data of \a type.
*/
bool QDSData::modify( QFile& data, const QMimeType& type )
{
    bool ret = false;
    if ( d->mUsingLocalStore ) {
        delete d->mLocalStore;
        d->mLocalStore = 0;
        delete d->mType;
        d->mType = 0;
        if ( ( d->mThreshold.mValue == -1 ) ||
             ( data.size() < d->mThreshold.mValue ) ) {
            if ( !data.isOpen() ) {
                if ( !data.open( QIODevice::ReadOnly ) ) {
                    qLog(DataSharing) << "QDSDataPrivate::QDSDataPrivate - "
                                      << "can't open file";
                }
            }
            d->mLocalStore = new QByteArray( data.readAll() );
            d->mType = new QMimeType( type );
            data.close();
            ret = true;
        } else {
            d->mUsingLocalStore = false;
            ret = QDSDataStore::add( d->mId, data, type );
        }
    } else if ( isValid() ) {
        ret = QDSDataStore::set( d->mId, data, type );
    }

    return ret;
}

/*!
    \fn void QDSData::deserialize(Stream &value)

    \internal

    Deserializes the QDSData instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QDSData::deserialize(Stream &stream)
{
    QLocalUniqueId id;
    bool usingLocalStore;
    QByteArray localdata;
    QString localtype;

    stream >> id;
    stream >> usingLocalStore;
    if ( usingLocalStore )
        stream >> localdata >> localtype;

    // Remove old d pointer, and create a new one
    delete d;

    if ( id != QLocalUniqueId() ) {
        if ( usingLocalStore ) {
            d = new QDSDataPrivate( id, localdata, QMimeType( localtype ) );
        } else {
            d = new QDSDataPrivate( id );

            // Decrement the transmit count, as we incremented it when it was
            // serialised.
            QDSDataStore::received( id );
        }
    } else {
        d = new QDSDataPrivate();
    }
}

/*!
    \fn void QDSData::serialize(Stream &value) const

    \internal

    Serializes the QDSData instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QDSData::serialize(Stream &stream) const
{
    stream << d->mId;
    stream << d->mUsingLocalStore;
    if ( d->mUsingLocalStore ) {
        stream << *(d->mLocalStore) << d->mType->id();
    } else {
        // Increment the transmit count, so we can guarantee it will exist when
        // the byte stream is used to recreate the object. We use transmit as
        // separate reference count, so that if a QDSData object is broadcast,
        // then the multiple receivers wont cause the data to be removed from the
        // store. This is not perfect, just less catastrophic. Really need QCop
        // to increment our reference counts for us...
        QDSDataStore::transmit( d->mId );
    }
}
