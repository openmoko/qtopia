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

#ifndef QDS_DATA_P_H
#define QDS_DATA_P_H

// Qtopia includes
#include <QLocalUniqueId>

// Qt includes
#include <QFile>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QByteArray;
class QDataStream;
class QMimeType;
class QBuffer;

// ============================================================================
//
//  QDSDataThrehold
//
// ============================================================================

class QDSDataThreshold
{
public:
    QDSDataThreshold();

    int mValue;
};

// ============================================================================
//
//  QDSLockedFile
//
// ============================================================================

class QDSLockedFile : public QFile
{
public:
    QDSLockedFile();
    explicit QDSLockedFile( const QString& name );
    ~QDSLockedFile();

    enum LockMode { ReadLock, WriteLock, NoLock };

    bool open ( QIODevice::OpenMode mode, bool acquireLock = true );
    void close();

    bool isLocked() const;
    bool lock( LockMode mode, bool block = true );
    LockMode lockMode() const;
    bool unlock();

    bool remove();

    static bool remove( const QString& name );

private:
    LockMode mLock;
};

// ============================================================================
//
//  QDSDataPrivate
//
// ============================================================================

class QDSDataPrivate
{
public:
    QDSDataPrivate();
    explicit QDSDataPrivate( const QLocalUniqueId& dataId );
    QDSDataPrivate( const QByteArray& data, const QMimeType& type );
    QDSDataPrivate( QFile& data, const QMimeType& type );
    QDSDataPrivate( const QLocalUniqueId& dataId,
                    const QByteArray& data,
                    const QMimeType& type );
    ~QDSDataPrivate();

    void shiftToStore();

    bool            mUsingLocalStore;
    QLocalUniqueId  mId;
    QByteArray*     mLocalStore;
    QMimeType*      mType;

    //static QDSDataThreshold   mThreshold;
    //static QUniqueIdGenerator mIdGen;
};

// ============================================================================
//
//  QDSDataStore
//
// ============================================================================

class QDSDataStore
{
public:
    static bool add( const QLocalUniqueId& id,
                     const QByteArray& data,
                     const QMimeType& type );

    static bool add( const QLocalUniqueId& id,
                     QFile& data,
                     const QMimeType& type );

    static bool add( const QLocalUniqueId& id );

    static bool set( const QLocalUniqueId& id,
                     const QByteArray& data );

    static bool set( const QLocalUniqueId& id,
                     QFile& data );

    static bool set( const QLocalUniqueId& id,
                     const QByteArray& data,
                     const QMimeType& type );

    static bool set( const QLocalUniqueId& id,
                     QFile& data,
                     const QMimeType& type );

    static bool remove( const QLocalUniqueId& id );
    static bool exists( const QLocalUniqueId& id );

    static bool transmit( const QLocalUniqueId& id );
    static bool received( const QLocalUniqueId& id );

    static QMimeType type( const QLocalUniqueId& id );
    static QByteArray data( const QLocalUniqueId& id );

    static QString infoFileName( const QLocalUniqueId& id );
    static QString dataFileName( const QLocalUniqueId& id );

    static int referenceCount( const QLocalUniqueId& id );
    static bool removeReference( const QLocalUniqueId& id );

    static int transmitCount( const QLocalUniqueId& id );
    static bool removeTransmit( const QLocalUniqueId& id );
};


#endif //QDS_DATA_P_H
