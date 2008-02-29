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

#include <QtopiaApplication>
#include <QStorageMetaInfo>
#include <QDebug>

int main( int argc, char **argv)
{
    QtopiaApplication app(argc, argv);
    QFileSystemFilter fsf;
    foreach ( QFileSystem *fs, QStorageMetaInfo::instance()->fileSystems(&fsf) ) {
        qWarning() << "Name : " << fs->name();
        qWarning() << "Disk : " << fs->disk();
        qWarning() << "Path : " << fs->path();
        qWarning() << "Options : " << fs->options();
        qWarning() << "Available blocks : " << fs->availBlocks();
        qWarning() << "Total blocks : " << fs->totalBlocks();
        qWarning() << "Block size : " << fs->blockSize();
        qWarning() << "Is removable : " << fs->isRemovable();
        qWarning() << "Is writeable : " << fs->isWritable();
        qWarning() << "Contains documents : " << fs->documents();
    }
    return 0;
}
