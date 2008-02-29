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

#include "locationcombo.h"

#include <qtopia/ir.h>
#include <qtopia/applnk.h>
#include <qtopia/global.h>
#include <qtopia/categorywidget.h>
#include <qtopia/categoryselect.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/filemanager.h>
#include <qtopia/config.h>
#include <qtopia/storage.h>
#include <qtopia/global.h>
#include <qtopia/qpemessagebox.h>

#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qsize.h>
#include <qcombobox.h>
#include <qregexp.h>

#include <qradiobutton.h>
#include <qlayout.h>

#include <stdlib.h>



LocationCombo::LocationCombo( QWidget *parent, const char *name )
    : QComboBox( FALSE, parent, name )
{
    storage = new StorageInfo;
    setLocation( 0 );
    connect( this, SIGNAL(activated(int)), this, SIGNAL(newPath()) );
    connect( storage, SIGNAL(disksChanged()), this, SLOT(updatePaths()) );
}

LocationCombo::LocationCombo( const AppLnk * lnk, QWidget *parent, const char *name )
    : QComboBox( FALSE, parent, name )
{
    storage = new StorageInfo;
    setLocation(lnk);
    connect( this, SIGNAL(activated(int)), this, SIGNAL(newPath()) );
    connect( storage, SIGNAL(disksChanged()), this, SLOT(updatePaths()) );
}

LocationCombo::~LocationCombo()
{
    delete storage;
}

void LocationCombo::setLocation( const AppLnk * lnk )
{
    originalLocation = -1;
    if ( lnk ) {
	QFileInfo fi( lnk->file() );
	fileSize = fi.size();
    } else {
	fileSize = 0;
    }

    setupCombo();

    if ( lnk ) {
	int n = locations.count();
	for ( int i = 0; i < n; i++ ) {
	    if ( lnk->file().contains( locations[i] ) )
		currentLocation = i;
	}
	originalLocation = currentLocation;
    }
    if ( currentLocation == -1 )
	currentLocation = 0; //default to the first one
    setCurrentItem( currentLocation );
}


void LocationCombo::setupCombo()
{
    clear();
    locations.clear();

    const QList<FileSystem> &fs = storage->fileSystems();
    QListIterator<FileSystem> it ( fs );
    QString s;
    QString homeDir = Global::homeDirPath();
    QString hardDiskHome;
    QString hardDiskPath;
    int index = 0;
    currentLocation = -1;
#if defined (_OS_LINUX_) || defined(Q_OS_LINUX)
    for ( ; it.current(); ++it ) {
	// we add 10k to the file size so we are sure we can also save the desktop file
	if ( (ulong)(*it)->availBlocks() * (ulong)(*it)->blockSize() > (ulong)fileSize + 10000 ) {
	    if ( (*it)->isRemovable() ||
		 (*it)->disk() == "/dev/mtdblock1" ||
		 (*it)->disk() == "/dev/mtdblock/1" ) {
		insertItem( (*it)->name(), index );
		locations.append( ((*it)->isRemovable() ? (*it)->path() : homeDir) );
		index++;
	    } else if ( (*it)->name().contains( "Hard Disk") &&
			homeDir.contains( (*it)->path() ) &&
			(*it)->path().length() > hardDiskPath.length() ) {
		hardDiskHome = (*it)->name();
		hardDiskPath = (*it)->path();
	    }
	}
    }
#else
    for ( ; it.current(); ++it ) {
	if ( (*it)->name().contains( "Hard Disk") &&
		homeDir.contains( (*it)->path() ) &&
			(*it)->path().length() > hardDiskPath.length() ) {
	    hardDiskHome = (*it)->name();
	    hardDiskPath = (*it)->path();
	}
    }
#endif
    if ( !hardDiskHome.isEmpty() ) {
	insertItem( hardDiskHome );
	QString hardDiskPath = homeDir + "/Documents";
	locations.append( hardDiskPath );
    }
}




void LocationCombo::updatePaths()
{
    bool wasChanged = isChanged();
    originalLocation = -1;
    QString oldPath = path();

    setupCombo();

    currentLocation = 0;
    int n = locations.count();
    for ( int i = 0; i < n; i++ ) {
	if ( oldPath == locations[i] ) {
	    currentLocation = i;
	    // if the path wasn't changed prior to this, then it
	    // still isn't changed now, even if it might have
	    // moved in the list.
	    if ( !wasChanged )
		originalLocation = currentLocation;
	}
    }
    setCurrentItem( currentLocation );
    if ( path() != oldPath )
	emit newPath();
}


bool LocationCombo::isChanged() const
{
    return currentItem() != originalLocation;
}


QString LocationCombo::path() const
{
    return locations[ currentItem() ];
}

QString LocationCombo::documentPath() const
{
    return path()+"/Documents";
}


const FileSystem *LocationCombo::fileSystem() const
{
    return storage->fileSystemOf( path() );
}




#if 0
void LocationCombo::apply()
{
}
#endif

