/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#define QTOPIA_INTERNAL_FILEOPERATIONS

#include "inlineedit.h"
#include "filebrowser.h"

#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/mimetype.h>
#include <qtopia/applnk.h>
#include <qtopia/services.h>
#include <qtopia/qcopenvelope_qws.h>

#ifdef QWS
#include <qcopchannel_qws.h>
#endif
#include <qmessagebox.h>
#include <qdir.h>
#include <qregexp.h>
#include <qheader.h>
#include <qtopia/qpetoolbar.h>
#include <qpopupmenu.h>
#include <qtopia/qpemenubar.h>
#include <qaction.h>
#include <qstringlist.h>
#include <qcursor.h>
#include <qmultilineedit.h>
#include <qfont.h>
#include <qtooltip.h>

#if defined(Q_OS_LINUX) || defined(_OS_LINUX_)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <sys/stat.h>

static int fileViewInstances = 0;
static QPixmap	*pmLockedFolder;
static QPixmap	*pmFolder;
static QPixmap	*pmLocked;
static QPixmap	*pmLibrary;
static QPixmap	*pmUnknown;

//
//  FileItem
//
FileItem::FileItem( QListView * parent, const QFileInfo & fi )
    : QListViewItem( parent ),
      fileInfo( fi )
{
    QDate d = fi.lastModified().date();

    setText( 0, fi.fileName() );
#ifndef QTOPIA_PHONE
    setText( 1, sizeString( fi.size() ) + " " );
    setText( 2, QString().sprintf("%4d-%02d-%02d",d.year(), d.month(), d.day() ) );
#endif

    MimeType mt(fi.filePath());

#ifndef QTOPIA_PHONE
    if( fi.isDir() )
	setText( 3, "directory" );
    else if( isLib() )
	setText( 3, "library" );
    else
	setText( 3, mt.description() );
#endif

    QPixmap pm;
    if( fi.isDir() ){
	if( !QDir( fi.filePath() ).isReadable() )
	    pm = *pmLockedFolder;
	else
	    pm = *pmFolder;
    }
    else if( !fi.isReadable() )
	pm = *pmLocked;
    else if( isLib() )
	pm = *pmLibrary;
    else
	pm = mt.pixmap();

    if ( pm.isNull() )
	pm = *pmUnknown;

    setPixmap(0,pm);
}

QString FileItem::sizeString( unsigned int s )
{
    double size = s;

    if ( size > 1024 * 1024 * 1024 )
		return QString().sprintf( "%.1f", size / ( 1024 * 1024 * 1024 ) ) + "G";
    else if ( size > 1024 * 1024 )
		return QString().sprintf( "%.1f", size / ( 1024 * 1024 ) ) + "M";
    else if ( size > 1024 )
		return QString().sprintf( "%.1f", size / ( 1024 ) ) + "K";
    else
		return QString::number( size ) + "B";
}

QString FileItem::key( int column, bool ascending ) const
{
    QString tmp;

    ascending = ascending;

    if( (column == 0) && fileInfo.isDir() ){ // Sort by name
		// We want the directories to appear at the top of the list
		tmp = (char) 0;
		return (tmp + text( column ).lower());
    }
    else if( column == 2 ) { // Sort by date
		QDateTime epoch( QDate( 1980, 1, 1 ) );
        tmp.sprintf( "%08d", epoch.secsTo( fileInfo.lastModified() ) );
		return tmp;
    }
    else if( column == 1 ) { // Sort by size
		return tmp.sprintf( "%08d", fileInfo.size() );
    }

    return text( column ).lower();
}

bool FileItem::isLib()
{
    // This is of course not foolproof
    if( !qstrncmp("lib", fileInfo.baseName(), 3) &&
		( fileInfo.extension().contains( "so" ) ||
		  fileInfo.extension().contains( "a" ) ) )
		return TRUE;
    else
		return FALSE;
}

int FileItem::launch()
{
    //
    // If this item is in $QPEDIR/Documents, pass it's
    // associated .desktop file along to the third party rather
    // than itself, to avoid double referencing the same file from
    // separate .desktop files.
    //
    QString	file_to_launch = fileInfo.filePath();
    MimeType	mt(fileInfo.filePath());
    QString	fname_desktop(file_to_launch);

    if (file_to_launch.contains(QDir::homeDirPath() + "/Documents")) {
	int	idx;

	idx = fname_desktop.findRev('.');           // fix suffix
	if (idx != -1) {
	    fname_desktop.truncate(idx);
	}
	fname_desktop += ".desktop";

	idx = fname_desktop.findRev('/');           // take filename only
	if (idx != -1) {
	    fname_desktop = fname_desktop.right(fname_desktop.length() - idx);
	}
	fname_desktop = QDir::homeDirPath() + "/Documents/" + mt.id() +
	       fname_desktop;
    }

    if (!QFile::exists(fname_desktop)) {
	//
	// We're trying to open a file that lives in $QPEDIR/Documents
	// but doesn't have an associated .desktop.  Something bad has
	// happened to get into this state; revert back to opening the
	// file we were originally asked for.
	//
	fname_desktop = fileInfo.filePath();
    }

    QString type = mt.id();
    if (isExecutable() && type.contains("application/octet-stream") ) {
        int cpid = fork();
        
        switch (cpid){
            case -1:
	        QMessageBox::warning( 0, QObject::tr( "File Manager" ),
                        QObject::tr( "<qt>Cannot fork this process.</qt>" ), QObject::tr( "&OK" ) );
                break;
            case 0:
                qDebug("Filebrowser: executing " + file_to_launch);
                execl(file_to_launch, file_to_launch, 0);
                break;
            default:
                //parent
                break;
        }
    } else {
        Config cfg(Service::appConfig("Open/"+type),Config::File);
        cfg.setGroup("Standard");
        if ( cfg.readNumEntry("Version") > 100 ) {
	    // Use Open service
            QCopEnvelope e(Service::channel("Open/"+type), "openFile(QString)");
	    e << fname_desktop;
        } else {
	    // Use setDocument()
	    DocLnk doc( fname_desktop, FALSE );
	    doc.execute();
        }
    }

    listView()->clearSelection();
    return 1;
}

bool FileItem::rename( const QString & name )
{
    QString oldpath, newpath;

    if ( name.isEmpty() )
		return FALSE;

    if ( name.contains( QRegExp("[/\\$\"\'\\*\\?]") ) )
		return FALSE;

    oldpath = fileInfo.filePath();
    newpath = fileInfo.dirPath() + "/" + name;

    if ( ::rename( oldpath.local8Bit().data(), newpath.local8Bit().data() ) != 0 )
		return FALSE;
    else
		return TRUE;
}

//
//  FileView
//
FileView::FileView( const QString & dir, QWidget * parent,
					const char * name )
    : QListView( parent, name ),
      menuTimer( this ),
      le( NULL ),
      itemToRename( NULL )
{
    setFrameStyle( NoFrame );
    addColumn( "Name" );
#ifndef QTOPIA_PHONE
    addColumn( "Date" );
    addColumn( "Size" );
    addColumn( "Type" );
#endif

    setMultiSelection( TRUE );
    header()->hide();

#ifndef QTOPIA_PHONE
    setColumnWidthMode( 0, Manual );
    setColumnWidthMode( 3, Manual );

    // right align yize column
    setColumnAlignment( 1, AlignRight );
#else
    setColumnWidthMode( 0, Maximum );
#endif

    if ( fileViewInstances++ == 0 )
	scaleIcons();
    generateDir( dir );

    connect( this, SIGNAL( clicked(QListViewItem*)),
			 SLOT( itemClicked(QListViewItem*)) );
    connect( this, SIGNAL( doubleClicked(QListViewItem*)),
			 SLOT( itemDblClicked(QListViewItem*)) );
    connect( this, SIGNAL( selectionChanged() ), SLOT( cancelMenuTimer() ) );
    connect( &menuTimer, SIGNAL( timeout() ), SLOT( showFileMenu() ) );
}

FileView::~FileView(void)
{
    if ( --fileViewInstances == 0 ) {
	delete pmLockedFolder;
	delete pmFolder;
	delete pmLocked;
	delete pmLibrary;
	delete pmUnknown;
    }
}

void FileView::resizeEvent( QResizeEvent *e )
{
#ifndef QTOPIA_PHONE
    setColumnWidth( 0, width() - 2 * lineWidth() - 20 - columnWidth( 1 ) - columnWidth( 2 ) );

    // hide type column, we use it for "sort by type" only
    setColumnWidth( 3, 0 );
    QListView::resizeEvent( e );
#else
    Q_UNUSED(e);
#endif
}

void FileView::updateDir()
{
    generateDir( currentDir );
}

void FileView::setDir( const QString & dir )
{
    if ( dir.startsWith( "/dev" ) ) {
	menuTimer.stop();
	QMessageBox::warning( this, tr( "File Manager" ),
			      tr( "<qt>Can't show /dev directory.</qt>" ), tr( "&OK" ) );
	return;
    }
    dirHistory += currentDir;
    generateDir( dir );
}

void FileView::generateDir( const QString & dir )
{
    QDir d( dir );

    if( d.exists() && !d.isReadable() ) return;

    //currentDir = d.canonicalPath();
    currentDir = d.path(); // store symlinks

    d.setFilter( QDir::Dirs | QDir::Files );
    d.setSorting( QDir::Name | QDir::DirsFirst | QDir::IgnoreCase |
				  QDir::Reversed );

    const QFileInfoList * list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    clear();
    while( (fi = it.current()) ){
		if( (fi->fileName() == ".") || (fi->fileName() == "..") ){
			++it;
			continue;
		}
		(void) new FileItem( (QListView *) this, *fi );
		++it;
    }

    emit dirChanged();
}

void FileView::rename()
{
    itemToRename = (FileItem *) currentItem();
    const QPixmap * pm;
    int pmw;

    if( itemToRename == NULL ) return;

    if( ( pm = itemToRename->pixmap( 0 ) ) == NULL )
		pmw = 0;
    else
		pmw = pm->width();

    ensureItemVisible( itemToRename );
    horizontalScrollBar()->setValue( 0 );
    horizontalScrollBar()->setEnabled( FALSE );
    verticalScrollBar()->setEnabled( FALSE );

    selected = isSelected( itemToRename );
    setSelected( itemToRename, FALSE );

    if( le == NULL ){
	le = new InlineEdit( this );
	le->setFrame( FALSE );
#ifndef QTOPIA_PHONE
	connect( le, SIGNAL( lostFocus() ), SLOT( endRenaming() ) );
#endif
	connect(le, SIGNAL(returnPressed()), this, SLOT(endRenaming()));
    }

    QRect r = itemRect( itemToRename );
    r.setTop( r.top() + frameWidth() + 1 );
    r.setLeft( r.left() + frameWidth() + pmw );
    r.setBottom( r.bottom() + frameWidth() );
    r.setWidth( columnWidth( 0 ) - pmw );

    le->setGeometry( r );
    le->setText( itemToRename->text( 0 ) );
    le->selectAll();
    le->show();
    le->setFocus();
}

void FileView::endRenaming()
{
    if( le && itemToRename ) {
	le->hide();
	setSelected( itemToRename, selected );

	if (itemToRename) {
	    if( !itemToRename->rename( le->text() ) ){
		QMessageBox::warning( this, tr( "Rename file" ),
		    tr( "Rename failed!" ), tr( "&OK" ) );
	    } else {
		updateDir();
	    }
	    itemToRename = NULL;
	    horizontalScrollBar()->setEnabled( TRUE );
	    verticalScrollBar()->setEnabled( TRUE );
	}
    }
}

void FileView::copy()
{
    endRenaming();

    // dont keep cut files any longer than necessary
    // ##### a better inmplementation might be to rename the CUT file
    // ##### to ".QPE-FILEBROWSER-MOVING" rather than copying it.
    QString sysCmd = QString("rm -rf %1qpemoving").arg(Global::tempDir());
    system ( sysCmd.local8Bit().data() );

    FileItem * i;

    if((i = (FileItem *) firstChild()) == 0) return;

    flist.clear();
    while( i ){
		if( i->isSelected() /*&& !i->isDir()*/ ){
			flist += i->getFilePath();
		}
		i = (FileItem *) i->nextSibling();
    }
}

void FileView::paste()
{
    int i, err;
    QString cmd, dest, basename, cd = currentDir;

    if(cd == "/") cd = "";

    for ( QStringList::Iterator it = flist.begin(); it != flist.end(); ++it ) {
		basename = (*it).mid((*it).findRev("/") + 1, (*it).length());

		dest = cd + "/" + basename;
		if( QFile( dest ).exists() ){
			i = 1;
			dest = cd + "/" + tr("Copy of %1").arg(basename);
			while( QFile( dest ).exists() ){
				dest = cd + "/" + tr("Copy (%1) of %2","number,filename").
				    arg(i++).arg(basename);
			}
		}

		//
		// Copy a directory recursively using the "cp" command -
		// may have to be changed
		//
		if( QFileInfo( (*it) ).isDir() ){
			cmd = "/bin/cp -fpR \"" + (*it) +"\" " + "\"" + dest + "\"";
			err = system( cmd.local8Bit().data() );
		} else if( !copyFile( dest, (*it) ) ){
			err = -1;
		} else {
			err = 0;
		}

		if ( err != 0 ) {
			QMessageBox::warning( this, tr("Paste file"), tr("Paste failed!"),
								  tr("OK") );
			break;
		} else {
			updateDir();
			QListViewItem * i = firstChild();
			basename = dest.mid( dest.findRev("/") + 1, dest.length() );

			while( i ){
				if( i->text(0) == basename ){
					setCurrentItem( i );
					ensureItemVisible( i );
					break;
				}
				i = i->nextSibling();
			}
		}
    }
}

bool FileView::copyFile( const QString & dest, const QString & src )
{
    char bf[ 50000 ];
    int  bytesRead;
    bool success = TRUE;
    struct stat status;

    QFile s( src );
    QFile d( dest );

    if( s.open( IO_ReadOnly | IO_Raw ) &&
		d.open( IO_WriteOnly | IO_Raw ) )
    {
		while( (bytesRead = s.readBlock( bf, sizeof( bf ) )) ==
			   sizeof( bf ) )
		{
			if( d.writeBlock( bf, sizeof( bf ) ) != sizeof( bf ) ){
				success = FALSE;
				break;
			}
		}
		if( success && (bytesRead > 0) ){
			d.writeBlock( bf, bytesRead );
		}
    } else {
		success = FALSE;
    }

#if !defined (Q_OS_WIN32)
    // Set file permissions
    if( stat( (const char *) src, &status ) == 0 ){
		chmod( (const char *) dest, status.st_mode );
    }

#else
    //#### revise set perms under WINNT?
#endif
    return success;
}

void FileView::cut()
{
    int err;
    // ##### a better inmplementation might be to rename the CUT file
    // ##### to ".QPE-FILEBROWSER-MOVING" rather than copying it.
    QString cmd, dest, basename;
    QString cd = QString("%1qpemoving").arg(Global::tempDir());

    endRenaming();

	QStringList newflist;
	newflist.clear();
	
	cmd = "rm -rf " + cd;
	system ( cmd.local8Bit().data() );
	cmd = "mkdir " + cd;
	system( cmd.local8Bit().data() );

// get the names of the files to cut
    FileItem * item;
 
    if((item = (FileItem *) firstChild()) == 0) return;
 
    flist.clear();
    while( item ){
        if( item->isSelected() /*&& !item->isDir()*/ ){
            flist += item->getFilePath();
        }
        item = (FileItem *) item->nextSibling();
    }

// move these files into a tmp dir
    for ( QStringList::Iterator it = flist.begin(); it != flist.end(); ++it ) {
        basename = (*it).mid((*it).findRev("/") + 1, (*it).length());
 
        dest = cd + "/" + basename;

		newflist += dest;
 
        cmd = "/bin/mv -f \"" + (*it) +"\" " + "\"" + dest + "\"";
	err = system( cmd.local8Bit().data() );
 
        if ( err != 0 ) {
            QMessageBox::warning( this, tr("Cut file"), tr("Cut failed!"),
                                  tr("OK") );
            break;
        } else {
            updateDir();
            QListViewItem * im = firstChild();
            basename = dest.mid( dest.findRev("/") + 1, dest.length() );
 
            while( im ){
                if( im->text(0) == basename ){
                    setCurrentItem( im );
                    ensureItemVisible( im );
                    break;
                }
                im = im->nextSibling();
            }
        }
    }

	// update the filelist to point to tmp dir so paste works nicely
	flist = newflist;
}

void FileView::del()
{
    FileItem * i;
    QStringList fl;
    QString cmd;
    int err;

    if((i = (FileItem *) firstChild()) == 0) return;

    while( i ){
		if( i->isSelected() ){
			fl += i->getFilePath();
		}
		i = (FileItem *) i->nextSibling();
    }
    if( fl.count() < 1 ) return;

    if( QMessageBox::warning( this, tr("Delete"), tr("Are you sure?"),
							  tr("Yes"), tr("No") ) == 0)
    {
		//
		// Dependant upon the "rm" command - will probably have to be replaced
		//
		for ( QStringList::Iterator it = fl.begin(); it != fl.end(); ++it ) {
			cmd = "/bin/rm -rf \"" + (*it) + "\"";
			err = system( cmd.local8Bit().data() );
			if ( err != 0 ) {
				QMessageBox::warning( this, tr("Delete"), tr("Delete failed!"),
									  tr("OK") );
				break;
			}
		}
		updateDir();
    }
}

void FileView::newFolder()
{
    int t = 1;
    FileItem * i;
    QString nd = currentDir + "/NewFolder";

    endRenaming();

    while( QFile( nd ).exists() ){
	nd.sprintf( "%s/NewFolder (%d)", (const char *) currentDir, t++ );
    }
    QDir d;
    if(d.mkdir( nd) == FALSE){
	QMessageBox::warning( this, tr( "New folder" ),
		tr( "<qt>Folder creation failed!</qt>" ),
		tr( "OK" ) );
	return;
    }
    updateDir();

    if((i = (FileItem *) firstChild()) == 0) return;

    while( i ){
	if( i->isDir() && ( i->getFilePath() == nd ) ){
	    setCurrentItem( i );
	    rename();
	    break;
	}
	i = (FileItem *) i->nextSibling();
    }
}

void FileView::viewAsText()
{
    FileItem * i = (FileItem *) currentItem();
    Config cfg(Service::appConfig("View/text/plain"),Config::File);
    cfg.setGroup("Standard");
    const char* msg = "setDocument(QString)";
    if ( cfg.readNumEntry("Version") > 100 )
	msg = "viewFile(QString)";
    QCopEnvelope e(Service::channel("View/text/*"), msg);
    e << i->getFilePath();
}

void FileView::itemClicked( QListViewItem * i)
{
    FileItem * t = (FileItem *) i;

    if( t == NULL ) return;
    if( t->isDir() ){
	setDir( t->getFilePath() );
    }
}

void FileView::itemDblClicked( QListViewItem * i)
{
    FileItem * t = (FileItem *) i;

    if(t == NULL) return;
    if(t->launch() == -1){
	QMessageBox::warning( this, tr( "Launch Application" ),
		tr( "Launch failed!" ), tr( "OK" ) );
    }
}

void FileView::parentDir()
{
    endRenaming();
    
    setDir( currentDir + "./.." );
}

void FileView::lastDir()
{
    endRenaming();
    if( dirHistory.count() == 0 ) return;

    QString newDir = dirHistory.last();
    dirHistory.remove( dirHistory.last() );
    generateDir( newDir );
}

void FileView::contentsMousePressEvent( QMouseEvent * e )
{
    QListView::contentsMousePressEvent( e );
    menuTimer.start( 750, TRUE );
}

void FileView::contentsMouseReleaseEvent( QMouseEvent * e )
{
    QListView::contentsMouseReleaseEvent( e );
    menuTimer.stop();
}

void FileView::cancelMenuTimer()
{
    if( menuTimer.isActive() )
		menuTimer.stop();
}

void FileView::addToDocuments()
{
    FileItem * i = (FileItem *) currentItem();
    DocLnk f;
    QString n = i->text(0);
    n.replace(QRegExp("\\..*"),"");
    f.setName( n );
    f.setFile( i->getFilePath() );
    f.writeLink();
}

void FileView::run()
{
    FileItem * i = (FileItem *) currentItem();
    i->launch();
}

void FileView::showFileMenu()
{
    FileItem * i = (FileItem *) currentItem();
    if ( !i )
		return;

    QPopupMenu * m = new QPopupMenu( this );

    MimeType mt(i->getFilePath());
    const AppLnk* app = mt.application();

    // Opening .desktop files is too ambiguous - disabled.
    // If the user *really* wants to touch those files, and take
    // responsibility for whatever mess they make, they can use Terminal.
    if ( !i->isDir() && i->getFilePath().right(8) != ".desktop" ) {
		m->insertItem( tr( "Add to Documents" ), this, SLOT( addToDocuments() ) );
		m->insertSeparator();

		if ( app )
			m->insertItem( app->pixmap(), tr("Open in %1","eg. text editor").arg(app->name()), this, SLOT( run() ) );
		else if( i->isExecutable() )
			m->insertItem( Resource::loadIconSet( i->text( 0 ) ), tr( "Run" ), this, SLOT( run() ) );

		m->insertItem( Resource::loadIconSet( "txt" ), tr( "View as text" ),
					   this, SLOT( viewAsText() ) );

		m->insertSeparator();
    }

    m->insertItem( tr( "Rename" ), this, SLOT( rename() ) );
    m->insertItem( Resource::loadIconSet("cut"),
				    tr( "Cut" ), this, SLOT( cut() ) );
    m->insertItem( Resource::loadIconSet("copy"),
				   tr( "Copy" ), this, SLOT( copy() ) );
    m->insertItem( Resource::loadIconSet("paste"),
				   tr( "Paste" ), this, SLOT( paste() ) );
    m->insertItem( tr( "Delete" ), this, SLOT( del() ) );
    m->insertSeparator();
    m->insertItem( tr( "Select all" ), this, SLOT( selectAll() ) );
    m->insertItem( tr( "Deselect all" ), this, SLOT( deselectAll() ) );
    m->popup( QCursor::pos() );
}

//
// Load and scale the pixmaps that we load manually, according to the
// iconsize that Applnk tells us.  We trust the size returned by AppLnk,
// as it's the same size that MimeTypeData uses.
//
void    FileView::scaleIcons(void)
{
    int	    iconsize = AppLnk::smallIconSize();
    QImage  img;

    pmLockedFolder = new QPixmap();
    pmFolder = new QPixmap();
    pmLocked = new QPixmap();
    pmLibrary = new QPixmap();
    pmUnknown = new QPixmap();

    img = Resource::loadImage("lockedfolder").smoothScale(iconsize, iconsize);
    pmLockedFolder->convertFromImage(img);

    img = Resource::loadImage("folder").smoothScale(iconsize, iconsize);
    pmFolder->convertFromImage(img);

    img = Resource::loadImage("locked").smoothScale(iconsize, iconsize);
    pmLocked->convertFromImage(img);

    img = Resource::loadImage("library").smoothScale(iconsize, iconsize);
    pmLibrary->convertFromImage(img);

    img = Resource::loadImage("qpe/UnknownDocument-14").
	smoothScale(iconsize, iconsize);
    pmUnknown->convertFromImage(img);
}

//
//  FileBrowser
//

FileBrowser::FileBrowser( QWidget * parent,
						  const char * name, WFlags f ) :
    QMainWindow( parent, name, f )
{
    init( QDir::current().canonicalPath() );
}

FileBrowser::FileBrowser( const QString & dir, QWidget * parent,
						  const char * name, WFlags f ) :
    QMainWindow( parent, name, f )
{
    init( dir );
}

void FileBrowser::init(const QString & dir)
{
    setCaption( tr("File Manager") );
    setIcon( Resource::loadPixmap( "FileBrowser" ) );
    setBackgroundMode( PaletteButton );

    fileView = new FileView( dir, this );
    fileView->setAllColumnsShowFocus( TRUE );

    setCentralWidget( fileView );
    setToolBarsMovable( FALSE );

    QPEToolBar* toolBar = new QPEToolBar( this );
    toolBar->setHorizontalStretchable( TRUE );

    QPEMenuBar* menuBar = new QPEMenuBar( toolBar );

    dirMenu = new QPopupMenu( this );
    menuBar->insertItem( tr( "Dir" ), dirMenu );

    sortMenu = new QPopupMenu( this );
    menuBar->insertItem( tr( "Sort" ), sortMenu );
    sortMenu->insertItem( tr( "by Name "), this, SLOT( sortName() ) );
    sortMenu->insertItem( tr( "by Size "), this, SLOT( sortSize() ) );
    sortMenu->insertItem( tr( "by Date "), this, SLOT( sortDate() ) );
    sortMenu->insertItem( tr( "by Type "), this, SLOT( sortType() ) );
    sortMenu->insertSeparator();
    sortMenu->insertItem( tr( "Ascending" ), this, SLOT( updateSorting() ) );
    sortMenu->setItemChecked( sortMenu->idAt( 5 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), TRUE );

    toolBar = new QPEToolBar( this );

    lastAction = new QAction( tr("Previous dir"), Resource::loadIconSet( "back" ),
							  QString::null, 0, this, 0 );
    connect( lastAction, SIGNAL( activated() ), fileView, SLOT( lastDir() ) );
    lastAction->addTo( toolBar );
    lastAction->setEnabled( FALSE );
    lastAction->setWhatsThis( tr("Changes location to the previous directory") );

    upAction = new QAction( tr("Parent dir"), Resource::loadIconSet( "up" ),
					 QString::null, 0, this, 0 );
    connect( upAction, SIGNAL( activated() ), fileView, SLOT( parentDir() ) );
    upAction->addTo( toolBar );
    upAction->setWhatsThis( tr("Changes location to the parent directory") );

    QAction *a = new QAction( tr("New folder"), Resource::loadIconSet( "newfolder" ),
					 QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( newFolder() ) );
    a->addTo( toolBar );
    a->setWhatsThis( tr("Creates a new folder") );

    a = new QAction( tr("Cut"), Resource::loadIconSet( "cut" ),
					 QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( cut() ) );
    a->addTo( toolBar );
    a->setWhatsThis( tr("Deletes the selected item") );

    a = new QAction( tr("Copy"), Resource::loadIconSet( "copy" ),
					 QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( copy() ) );
    a->addTo( toolBar );
    a->setWhatsThis( tr("Copies the selected item") );

    pasteAction = new QAction( tr("Paste"), Resource::loadIconSet( "paste" ),
							   QString::null, 0, this, 0 );
    connect( pasteAction, SIGNAL( activated() ), fileView, SLOT( paste() ) );
    pasteAction->addTo( toolBar );
    pasteAction->setWhatsThis( tr("Paste the contents of the last copy or cut operation") );

    connect( fileView, SIGNAL( dirChanged() ), SLOT( updateDirMenu() ) );
    updateDirMenu();

#ifndef QT_NO_COP
    QCopChannel* pcmciaChannel = new QCopChannel( "QPE/Card", this );
    connect( pcmciaChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	 this, SLOT(pcmciaMessage(const QCString&,const QByteArray&)) );
#endif
}

void FileBrowser::pcmciaMessage( const QCString &msg, const QByteArray &)
{
    if ( msg == "mtabChanged()" ) {
	// ## Only really needed if current dir is on a card
        fileView->updateDir();
    }
}

void FileBrowser::dirSelected( int id )
{
    int i = 0, j;
    QString dir;

    // Bulid target dir from menu
    while( (j = dirMenu->idAt( i )) != id ){
		dir += dirMenu->text( j ).stripWhiteSpace();
		if( dirMenu->text( j ) != "/" ) dir += "/";
		i++;
    }
    dir += dirMenu->text( dirMenu->idAt( i ) ).stripWhiteSpace();

    fileView->setDir( dir );
}

void FileBrowser::updateDirMenu()
{
    QString spc, cd = fileView->cd();
    QStringList l = QStringList::split( "/", cd );
    int i = 0;

    dirMenu->clear();
    dirMenu->insertItem( tr( "/" ), this, SLOT( dirSelected(int) ) );

    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
		spc.fill( ' ', i++);
		dirMenu->insertItem( spc + (*it), this,
							 SLOT( dirSelected(int) ) );
    }
    dirMenu->setItemChecked( dirMenu->idAt( l.count() ), TRUE );

    lastAction->setEnabled( fileView->history().count() != 0 );
    upAction->setEnabled( cd != "/" );
}

void FileBrowser::sortName()
{
    fileView->setSorting( 0, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), FALSE );
}

void FileBrowser::sortSize()
{
    fileView->setSorting( 1, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), FALSE );
}

void FileBrowser::sortDate()
{
    fileView->setSorting( 2, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), FALSE );
}

void FileBrowser::sortType()
{
    fileView->setSorting( 3, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), TRUE );
}

void FileBrowser::updateSorting()
{
    sortMenu->setItemChecked( sortMenu->idAt( 5 ), !sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );

    if ( sortMenu->isItemChecked( sortMenu->idAt( 0 ) ) )
		sortName();
    else if ( sortMenu->isItemChecked( sortMenu->idAt( 1 ) ) )
		sortSize();
    else if ( sortMenu->isItemChecked( sortMenu->idAt( 2 ) ) )
		sortDate();
    else
		sortType();
}
