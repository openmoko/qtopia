/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/storage.h>
#ifdef QT_QWS_CUSTOM
#include <qtopia/custom.h>
#endif

#include <qtimer.h>
#include <qfile.h>
#ifdef QWS
#include <qcopchannel_qws.h>
#endif

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
#include <unistd.h>
#include <sys/vfs.h>
#include <mntent.h>
#endif

#include <qstringlist.h>

static bool isCF(const QString& m)
{
    FILE* f = fopen("/var/run/stab", "r");
    if (!f) f = fopen("/var/state/pcmcia/stab", "r");
    if (!f) f = fopen("/var/lib/pcmcia/stab", "r");
    if ( f ) {
	char line[1024];
	char devtype[80];
	char devname[80];
	while ( fgets( line, 1024, f ) ) {
	    // 0       ide     ide-cs  0       hda     3       0
	    if ( sscanf(line,"%*d %s %*s %*s %s", devtype, devname )==2 )
	    {
		if ( QString(devtype) == "ide" && m.find(devname)>0 ) {
		    fclose(f);
		    return TRUE;
		}
	    }
	}
	fclose(f);
    }
    return FALSE;
}

/*! \class StorageInfo storage.h
  \brief The StorageInfo class describes the disks mounted on the file system.

  This class provides access to the mount information for the Linux
  filesystem. Each mount point is represented by the FileSystem class.
  To ensure this class has the most up to date size information, call
  the update() method. Note that this will automatically be signaled
  by the operating system when a disk has been mounted or unmounted.

  \ingroup qtopiaemb
*/

/*! Constructor that determines the current mount points of the filesystem.
  The standard \a parent parameters is passed on to QObject.
 */
StorageInfo::StorageInfo( QObject *parent )
    : QObject( parent )
{
    mFileSystems.setAutoDelete( TRUE );
#ifndef QT_NO_COP
    channel = new QCopChannel( "QPE/Card", this );
    connect( channel, SIGNAL(received(const QCString &, const QByteArray &)),
	     this, SLOT(cardMessage( const QCString &, const QByteArray &)) );
#endif
    update();
}

/*! Returns the longest matching FileSystem that starts with the
   same prefix as \a filename as its mount point.
*/
const FileSystem *StorageInfo::fileSystemOf( const QString &filename )
{
#if 0
    // The filesystem way of doing things...
    QString existingfilename = filename;
    struct stat st;
    while (stat(QFile::encodeName(existingfilename),&st)) {
	int x = existingfilename.findRev('/');
	if ( x < 0 )
	    return 0;
	if ( x )
	    existingfilename.truncate(x-1);
	else
	    break;
    }
    int devno = st.st_dev;
    for (QListIterator<FileSystem> i(mFileSystems); i.current(); ++i) {
	if ( 0==stat(QFile::encodeName((*i)->path()),&st) ) {
	    if ( st.st_dev == devno ) {
		return *i;
	    }
	}
    }
    return 0;
#else
    // The filename way (doesn't understand symlinks)
    FileSystem *bestMatch = 0;
    uint bestLen = 0;
    uint currLen;

    for (QListIterator<FileSystem> i(mFileSystems); i.current(); ++i) {
	if ( filename.startsWith( (*i)->path() ) ) {
	    currLen = (*i)->path().length();
	    if ( currLen == 1 )
		currLen = 0; // Fix checking '/' root mount which is a special case
	    if ( filename.length() == currLen ||
	         filename[(int)currLen] == '/' ||
	         filename[(int)currLen] == '\\') {
		if ( currLen > bestLen || bestLen == 0 ) {
		    bestMatch = (*i);
		    bestLen = currLen;
		}
	    }
	}
    }
    return bestMatch;
#endif
}


void StorageInfo::cardMessage( const QCString& msg, const QByteArray& )
{
    if ( msg == "mtabChanged()" )
	update();
}

/*! Updates the mount and free space available information for each mount
  point. This method is automatically called when a disk is mounted or
  unmounted.
*/
void StorageInfo::update()
{
    //qDebug("StorageInfo::updating");
#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
    struct mntent *me;
    FILE *mntfp = setmntent( "/etc/mtab", "r" );

    QStringList curdisks;
    QStringList curopts;
    QStringList curfs;
    bool rebuild = FALSE;
    int n=0;
    if ( mntfp ) {
	while ( (me = getmntent( mntfp )) != 0 ) {
	    QString fs = me->mnt_fsname;
	    if ( fs.left(7)=="/dev/hd" || fs.left(7)=="/dev/sd"
		    || fs.left(8)=="/dev/mtd"
		    || fs.left(9) == "/dev/mmcd"
			    // "which-qtopia" may be running off the SD card
			    && (QString(me->mnt_dir)!="/home/QtPalmtop"
			        && QString(me->mnt_dir)!="/opt/Qtopia")
		)
	    {
		n++;
		curdisks.append(fs);
		curopts.append( me->mnt_opts );
		//qDebug("-->fs %s opts %s", fs.latin1(), me->mnt_opts );
		curfs.append( me->mnt_dir );
		bool found = FALSE;
		for (QListIterator<FileSystem> i(mFileSystems); i.current(); ++i) {
		    if ( (*i)->disk() == fs ) {
			found = TRUE;
			break;
		    }
		}
		if ( !found )
		    rebuild = TRUE;
	    }
	}
	endmntent( mntfp );
    }
    if ( rebuild || n != (int)mFileSystems.count() ) {
	mFileSystems.clear();
	QStringList::ConstIterator it=curdisks.begin();
	QStringList::ConstIterator fsit=curfs.begin();
	QStringList::ConstIterator optsIt=curopts.begin();
	for (; it!=curdisks.end(); ++it, ++fsit, ++optsIt) {
	    QString opts = *optsIt;

	    QString disk = *it;
	    QString humanname;
	    bool removable = FALSE;
	    if ( isCF(disk) ) {
		humanname = tr("CF Card");
		removable = TRUE;
	    } else if ( disk == "/dev/hda1" ) {
		humanname = tr("Hard Disk");
	    } else if ( disk.left(9) == "/dev/mmcd" ) {
		humanname = tr("SD Card");
		removable = TRUE;
	    } else if ( disk.left(7) == "/dev/hd" )
		humanname = tr("Hard Disk") + " " + humanname.mid(7);
	    else if ( disk.left(7) == "/dev/sd" )
		humanname = tr("SCSI Hard Disk") + " " + humanname.mid(7);
	    else if ( disk == "/dev/mtdblock1" || humanname == "/dev/mtdblock/1" )
		humanname = tr("Internal Storage");
	    else if ( disk.left(14) == "/dev/mtdblock/" )
		humanname = tr("Internal Storage") + " " + humanname.mid(14);
	    else if ( disk.left(13) == "/dev/mtdblock" )
		humanname = tr("Internal Storage") + " " + humanname.mid(13);
	    FileSystem *fs = new FileSystem( disk, *fsit, humanname, removable, opts );
	    mFileSystems.append( fs );
	}
	emit disksChanged();
    } else {
	// just update them
	for (QListIterator<FileSystem> i(mFileSystems); i.current(); ++i)
	    i.current()->update();
    }
#endif
}

/*! \fn const QList<FileSystem> &StorageInfo::fileSystems() const
  Returns a list of all available mounted file systems.

  \warning This may change in Qtopia 3.x to return only relevant Qtopia file systems (and ignore mount points such as /tmp)
*/

/*! \fn void StorageInfo::disksChanged()
  Gets emitted when a disk has been mounted or unmounted, such as when
  a CF card has been inserted.
*/

//---------------------------------------------------------------------------

FileSystem::FileSystem( const QString &disk, const QString &path, const QString &name, bool rem, const QString &o )
    : fsdisk( disk ), fspath( path ), humanname( name ), blkSize(512), totalBlks(0), availBlks(0), removable( rem ), opts( o )
{
    update();
}

void FileSystem::update()
{
#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
    struct statfs fs;
    if ( !statfs( fspath.latin1(), &fs ) ) {
	blkSize = fs.f_bsize;
	totalBlks = fs.f_blocks;
	availBlks = fs.f_bavail;
    } else {
	blkSize = 0;
	totalBlks = 0;
	availBlks = 0;
    }
#endif
}

/*! \class FileSystem storage.h
  \brief The FileSystem class describes a single mount point.

  This class simply returns information about a mount point, including
  file system name, mount point, human readable name, size information
  and mount options information.
  \ingroup qtopiaemb

  \sa StorageInfo
*/

/*! \fn const QString &FileSystem::disk() const
  Returns the file system name, such as /dev/hda3
*/

/*! \fn const QString &FileSystem::path() const
  Returns the mount path, such as /home
*/

/*! \fn const QString &FileSystem::name() const
  Returns the translated, human readable name for the mount directory.
*/

/*! \fn const QString &FileSystem::options() const
  Returns the mount options
*/

/*! \fn long FileSystem::blockSize() const
  Returns the size of each block on the file system.
*/

/*! \fn long FileSystem::totalBlocks() const
  Returns the total number of blocks on the file system
*/

/*! \fn long FileSystem::availBlocks() const
  Returns the number of available blocks on the file system
 */

/*! \fn bool FileSystem::isRemovable() const
  Returns flag whether the file system can be removed, such as a CF card
  would be removable, but the internal memory wouldn't
*/

/*! \fn bool FileSystem::isWritable() const
  Returns flag whether the file system is mounted as writable or read-only.
  Returns FALSE if read-only, TRUE if read and write.
*/

/*! \fn QStringList StorageInfo::fileSystemStrings() const
  Returns a list of filesystem names.
*/
