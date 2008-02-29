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
#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/storage.h>
#include <qtopia/categoryselect.h>
#include <qpainter.h>
#include <qimage.h>
#include <qcopchannel_qws.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qlistview.h>
#include <qlist.h>
#include <qpixmap.h>

#include "mediaplayerstate.h"
#include "playlistselection.h"
#include "fileinfo.h"

#include <stdlib.h>


class PlayListSelectionItem : public QListViewItem {
public:
    PlayListSelectionItem( QListView *parent, const DocLnk &f ) : QListViewItem( parent ) {
	setFile( f );
    }

    const DocLnk &file() const { return fl; }

    void setFile( const DocLnk& f ) {
	fl = f;

	QFile file( f.file() );

	// First column
	if ( file.exists() )
	    setPixmap( 0, f.pixmap() );
	else
	    setPixmap( 0, Resource::loadPixmap("UnknownDocument") );
	setText( 0, f.name().lower() );

	// Second Column
	StorageInfo storage;
	const FileSystem *fileSystem = storage.fileSystemOf( f.file() );
	if ( fileSystem )
	    setText( 1, fileSystem->name() );
	else
	    setText( 1, "-" );

	// Third Column
	if ( file.exists() ) {
	    uint fileSize = file.size();
	    QString sizeText = "0B";
	    if ( fileSize < 1000 )
		sizeText = QString("%1B").arg( fileSize );
	    else if ( fileSize < 1000000 )
		sizeText = QString("%1KB").arg( fileSize / 1000 );
	    else 
		sizeText = QString("%1MB").arg( (double)fileSize / 1000000.0, 0, 'f', 1 );
	    setText( 2, sizeText );
	} else {
	    setText( 2, "-" );
	}
/*
	setText( 3, f.type() );

	QDate date = QFileInfo( file ).lastModified().date();
	setText( 4, QString("%1 %1 %1").arg( date.monthName( date.month() ) ).arg( date.day() ).arg( date.year() ) );
*/
    }

    void paintCell( QPainter *p, const QColorGroup & cg, int column, int width, int alignment ) {
	if ( column == 0 ) {
	    bool updates = listView()->isUpdatesEnabled();
	    listView()->setUpdatesEnabled( false );
	    setText( 0, fl.name() );
	    QListViewItem::paintCell( p, cg, column, width, alignment );
	    setText( 0, fl.name().lower() );
	    listView()->setUpdatesEnabled( updates );
	} else {
	    QListViewItem::paintCell( p, cg, column, width, alignment );
	}
	
    }

private:
    DocLnk fl;
};


class PlayListSelectionData {
    public:
	CategorySelect		*catSel;
	QComboBox		*locSel;
	QList<QListViewItem>	shuffledList;
};


PlayListSelection::PlayListSelection( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    d = new PlayListSelectionData;

    d->catSel = 0;
    d->locSel = 0;

    QPEApplication::setStylusOperation( viewport(), QPEApplication::RightOnHold );

    setAllColumnsShowFocus( TRUE );
    setFrameStyle( NoFrame );

    addColumn( tr( "Title" ) );
    addColumn( tr( "Media" ) );
    addColumn( tr( "Size" ) );

    setColumnAlignment( 0, Qt::AlignLeft );
    setColumnAlignment( 1, Qt::AlignLeft );
    setColumnAlignment( 2, Qt::AlignRight );

    setColumnWidthMode( 0, QListView::Manual );
    setColumnWidthMode( 1, QListView::Manual );
    setColumnWidthMode( 2, QListView::Manual );

    setColumnWidth( 0, 105 );
    setColumnWidth( 1, 60 );
    setColumnWidth( 2, 50 );

    connect( this, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint &, int ) ), this, SLOT( showItemProperties( QListViewItem * ) ) );

    clear();

    sbVis = FALSE;
}


PlayListSelection::~PlayListSelection()
{
    delete d;
}


PlayListSelectionItem *PlayListSelection::newItem( const DocLnk& lnk )
{
    PlayListSelectionItem *item = new PlayListSelectionItem( this, lnk );
    unsigned int randomPos = (unsigned int)((double)rand() * d->shuffledList.count() / RAND_MAX);
    randomPos = QMIN( randomPos, d->shuffledList.count() );
    d->shuffledList.insert( randomPos, item );
    if ( verticalScrollBar()->isVisible() != sbVis )
	viewportResizeEvent( 0 );
    return item;
}


void PlayListSelection::deleteItem( PlayListSelectionItem *item )
{
    d->shuffledList.remove( item );
    delete item;
}


// Maximize the 1st column to fill the space left from the next two columns
// Attempt to resize so no horizontal scroll bar is needed. Perhaps need to catch
// other changes like font size changes or style changes to really do this properly,
// but at the very least try to get the desired columns on the screen.
void PlayListSelection::viewportResizeEvent( QResizeEvent * )
{
    QScrollBar *sb = verticalScrollBar();
    sbVis = sb->isVisible();
    int sbWidth = sbVis ? sb->width() + 1 : 0;
    setColumnWidth( 0, width() - columnWidth( 1 ) - columnWidth( 2 ) - sbWidth - 2 * lineWidth() );
}


void PlayListSelection::showItemProperties( QListViewItem *i )
{
    PlayListSelectionItem *item = (PlayListSelectionItem *)i;
    if ( item ) {
	MediaFileInfoWidget *infoWidget = new MediaFileInfoWidget( item->file(), 0 );
	QPEApplication::execDialog( infoWidget );
    }
}


void PlayListSelection::setCategoryFilter( CategorySelect *c )
{
    d->catSel = c;
}


void PlayListSelection::setLocationFilter( QComboBox *l )
{
    d->locSel = l;
}


bool PlayListSelection::filtersMatch( const DocLnk& lnk )
{
    if ( d->catSel && !d->catSel->containsCategory( lnk.categories() ) )
	return FALSE;
    if ( d->locSel ) {
	StorageInfo storage;
	QString comboLoc = d->locSel->currentText();
        if ( comboLoc == tr( "All Locations" ) )
	    return TRUE;
	const FileSystem *fileSystem = storage.fileSystemOf( lnk.file() );
        if ( fileSystem && comboLoc == fileSystem->name() )
	    return TRUE;
	return FALSE;
    }
    return TRUE;
}


void PlayListSelection::removeLink( const DocLnk &lnk )
{
    QListViewItem *prevItem = 0;
    QListViewItem *item = firstChild();
    while ( item ) {
	PlayListSelectionItem *i = (PlayListSelectionItem *)item;
	DocLnk l = i->file();
	if ( l.name() == lnk.name() && l.file() == lnk.file() ) {
	    deleteItem( i );
	    return;
	}
	prevItem = item;
	item = item->nextSibling();
    }
}


void PlayListSelection::changeLink( const DocLnk &oldDoc, const DocLnk &newDoc )
{
    QListViewItem *item = firstChild();
    while ( item ) {
	PlayListSelectionItem *i = (PlayListSelectionItem *)item;
	DocLnk l = i->file();
	if ( l.name() == oldDoc.name() && l.file() == oldDoc.file() ) {
	    if ( !filtersMatch( newDoc ) ) 
		// The category has been changed to no longer be in the selected category
		deleteItem( i );
	    else 
		i->setFile( newDoc );
	    return;
	}
	item = item->nextSibling();
    }
}


void PlayListSelection::addLink( const DocLnk &lnk )
{
    // Ignore if it is not in the category
    if ( !filtersMatch( lnk ) )
	return;
    newItem( lnk );
}


void PlayListSelection::contentsMouseMoveEvent( QMouseEvent *event )
{
    if ( event->state() == QMouseEvent::LeftButton ) {
	QListViewItem *currentItem = selectedItem();
	QListViewItem *itemUnder = itemAt( QPoint( event->pos().x(), event->pos().y() - contentsY() ) );
	if ( currentItem && currentItem->itemAbove() == itemUnder )
	    moveSelectedUp();
	else if ( currentItem && currentItem->itemBelow() == itemUnder )
	    moveSelectedDown();
    }
}


const DocLnk *PlayListSelection::current() 
{
    PlayListSelectionItem *item = (PlayListSelectionItem *)selectedItem();
    if ( item )
	return &(item->file());
    if ( firstChild() ) { // try harder
	setSelected( firstChild(), TRUE );
	item = (PlayListSelectionItem *)firstChild();
	if ( item )
	    return &(item->file());
    }
    return NULL;
}


void PlayListSelection::addToSelection( const DocLnk &lnk )
{
    PlayListSelectionItem *item = newItem( lnk );
    QListViewItem *current = selectedItem();
    if ( current )
        item->moveItem( current );
    setSelected( item, TRUE );
    ensureItemVisible( selectedItem() );
}


void PlayListSelection::removeSelected()
{
    QListViewItem *item = selectedItem();
    if ( item )
	deleteItem( (PlayListSelectionItem *)item );
    setSelected( currentItem(), TRUE );
    ensureItemVisible( selectedItem() );
}


void PlayListSelection::moveSelectedUp()
{
    QListViewItem *item = selectedItem();
    if ( item && item->itemAbove() )
	item->itemAbove()->moveItem( item );
    ensureItemVisible( selectedItem() );
}


void PlayListSelection::moveSelectedDown()
{
    QListViewItem *item = selectedItem();
    if ( item && item->itemBelow() )
        item->moveItem( item->itemBelow() );
    ensureItemVisible( selectedItem() );
}


bool PlayListSelection::prev()
{
    QListViewItem *curItem = selectedItem();
    QListViewItem *prevItem = 0;
    if ( mediaPlayerState->shuffled() ) {
	d->shuffledList.find( curItem );
	prevItem = d->shuffledList.prev();
    } else if ( curItem )
	prevItem = curItem->itemAbove();
    if ( !prevItem ) {
	if ( mediaPlayerState->looping() ) {
	    if ( mediaPlayerState->shuffled() ) {
		prevItem = d->shuffledList.last();
	    } else {
		return last();
	    }
	} else
	    return FALSE;
    }
    setSelected( prevItem, TRUE );
    ensureItemVisible( prevItem );
    return TRUE;
}


bool PlayListSelection::next()
{
    QListViewItem *curItem = selectedItem();
    QListViewItem *nextItem = 0;
    if ( mediaPlayerState->shuffled() ) {
	d->shuffledList.find( curItem );
	nextItem = d->shuffledList.next();
    } else if ( curItem )
	nextItem = curItem->itemBelow();
    if ( !nextItem ) {
	if ( mediaPlayerState->looping() ) {
	    if ( mediaPlayerState->shuffled() ) {
		nextItem = d->shuffledList.first();
	    } else {
		return first();
	    }
	} else
	    return FALSE;
    }
    setSelected( nextItem, TRUE );
    ensureItemVisible( nextItem );
    return TRUE;
}


bool PlayListSelection::advanceSelection()
{
    QListViewItem *curItem = selectedItem();
    if ( curItem ) {
	QListViewItem *nextItem = curItem->itemBelow();
	if ( nextItem ) {
	    setSelected( nextItem, TRUE );
	    ensureItemVisible( nextItem );
	    return TRUE;
	}
    }
    return FALSE;
}


bool PlayListSelection::first()
{
    QListViewItem *item = firstChild();
    if ( item )
        setSelected( item, TRUE );
    else
	return FALSE;
    ensureItemVisible( selectedItem() );
    return TRUE;
}


bool PlayListSelection::last()
{
    QListViewItem *prevItem = NULL;
    QListViewItem *item = firstChild();
    while ( ( item = item->nextSibling() ) )
	prevItem = item;
    if ( prevItem )
        setSelected( prevItem, TRUE );
    else
	return FALSE;
    ensureItemVisible( selectedItem() );
    return TRUE;
}


