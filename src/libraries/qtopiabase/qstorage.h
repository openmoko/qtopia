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
#ifndef __storage_h__
#define __storage_h__

#include <qtopiaglobal.h>
#include <qobject.h>
#include <qlist.h>

class QFileSystem;
class QFileSystemFilter;
class QtopiaChannel;

#if defined (QTOPIA_TEMPLATEDLL)
//MOC_SKIP_BEGIN
template class QTOPIABASE_EXPORT QList<QFileSystem*>;
//MOC_SKIP_END
#endif


class QTOPIABASE_EXPORT QStorageMetaInfo : public QObject
{
    Q_OBJECT
public:
    explicit QStorageMetaInfo( QObject *parent=0 );
    ~QStorageMetaInfo();

    QList<QFileSystem*> fileSystems( QFileSystemFilter *filter, bool connectedOnly=true );
    const QFileSystem *fileSystemOf( const QString &filename, bool connectedOnly=true );
    QStringList fileSystemNames( QFileSystemFilter *filter, bool connectedOnly=true ); // libqtopia
    QString cardInfoString();
    QString installLocationsString();

signals:
    void disksChanged();

public slots:
    void update();

private slots:
    void cardMessage( const QString& msg, const QByteArray& data );
private:
    QString infoString( QList<QFileSystem*> filesystems, const QString &extension );
    QList<QFileSystem*> mFileSystems;
    QtopiaChannel *channel;
};


class QTOPIABASE_EXPORT QFileSystem
{
public:
    const QString &disk() const { return mDisk; }
    const QString &path() const { return mPath; }
    const QString &prevPath() const { return mPrevPath; }
    const QString &options() const { return mOptions; }
    const QString &name() const { return mName; }
    bool isRemovable() const { return mRemovable; }
    bool applications() const { return mApplications; }
    bool documents() const { return mDocuments; }
    bool contentDatabase() const { return mContentDatabase; }
    bool isConnected() const { return mConnected; }

    long blockSize() const { return mBlockSize; }
    long totalBlocks() const { return mTotalBlocks; }
    long availBlocks() const { return mAvailBlocks; }

    bool isWritable() const { return mOptions.contains("rw"); }

private:
    friend class QStorageMetaInfo;
    QFileSystem( const QString &disk, const QString &path, const QString &prevPath, const QString &options,
                const QString &name, bool removable, bool applications, bool documents,
                bool contentDatabase, bool connected);
    ~QFileSystem() {};

    void update();

    QString mDisk;
    QString mPath;
    QString mPrevPath;
    QString mOptions;
    QString mName;
    bool mRemovable;
    bool mApplications;
    bool mDocuments;
    bool mContentDatabase;
    bool mConnected;

    long mBlockSize;
    long mTotalBlocks;
    long mAvailBlocks;
};


class QTOPIABASE_EXPORT QFileSystemFilter
{
public:
    enum FilterOption { Either, Set, NotSet };

    QFileSystemFilter();
    virtual ~QFileSystemFilter();

    virtual bool filter( QFileSystem *fs );

    FilterOption documents;
    FilterOption applications;
    FilterOption removable;
    FilterOption content;
};


#endif
