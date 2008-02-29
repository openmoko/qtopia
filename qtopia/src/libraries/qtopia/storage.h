/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __storage_h__
#define __storage_h__

#include <qtopia/qpeglobal.h>
#include <qobject.h>
#include <qlist.h>

class FileSystem;
#ifndef QT_NO_COP
class QCopChannel;
#endif


#if defined (QTOPIA_TEMPLATEDLL)
//MOC_SKIP_BEGIN
template class QTOPIA_EXPORT QList<FileSystem>;
//MOC_SKIP_END
#endif

class QTOPIA_EXPORT StorageInfo : public QObject
{
    Q_OBJECT
public:
    StorageInfo( QObject *parent=0 );

    const QList<FileSystem> &fileSystems() const { return mFileSystems; }
    const FileSystem *fileSystemOf( const QString &filename );
    QStringList fileSystemNames() const; // libqtopia

signals:
    void disksChanged();

public slots:
    void update();

private slots:
    void cardMessage( const QCString& msg, const QByteArray& data );
private:
    QList<FileSystem> mFileSystems;
#ifndef QT_NO_COP
    QCopChannel *channel;
#endif
};

class QTOPIA_EXPORT FileSystem
{
public:
    const QString &disk() const { return fsdisk; }
    const QString &path() const { return fspath; }
    const QString &name() const { return humanname; }
    const QString &options() const { return opts; }
    long blockSize() const { return blkSize; }
    long totalBlocks() const { return totalBlks; }
    long availBlocks() const { return availBlks; }
    bool isRemovable() const { return removable; }
    bool isWritable() const { return opts.contains("rw"); }

private:
    friend class StorageInfo;
    FileSystem( const QString &disk, const QString &path, const QString &humanname, bool rem, const QString &opts );
    void update();

    QString fsdisk;
    QString fspath;
    QString humanname;
    long blkSize;
    long totalBlks;
    long availBlks;
    bool removable;
    QString opts;
};


#endif
