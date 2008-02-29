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
#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/storage.h>
#include <qtopia/categoryselect.h>
#include <qtopia/categorydialog.h>
#include <qtopia/resource.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qheader.h>

#include "mediaplayerstate.h"
#include "mediaselector.h"
#include "fileinfo.h"

#include <stdlib.h>

#define CHECK_BOX_WIDTH	    14


/*!
  \class MediaSelector mediaselector.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class lists MediaSelectorItems. Media items could
    be listed by various media specific attributes, id3 tags or
    other criteria. Phone version only lists the document name
    due to space restrictions.
*/


MediaSelectorItem::MediaSelectorItem( QListView *parent, const DocLnk &f )
    : QCheckListItem( parent, "", QCheckListItem::CheckBox ), selectable( true )
{
    setFile( f );
}


const DocLnk &MediaSelectorItem::file() const
{
    return fl;
}


void MediaSelectorItem::setFile( const DocLnk& f )
{
    fl = f;
    setPixmap( 1, f.pixmap() );
    setText( 1, f.name() );
}


QString MediaSelectorItem::key(int column, bool ascending) const
{
    switch ( column ) {
	case 1:
	    return fl.name().lower();
	default:
	    break;
    }
    return QListViewItem::key(column, ascending);
}

/*
static QColor mixColors(const QColor& a, const QColor& b, int pcth)
{
    int pcthb = 100 - pcth;
    return QColor(
	    (a.red() * pcth + b.red() * pcthb) / 100,
	    (a.green() * pcth + b.green() * pcthb) / 100,
	    (a.blue() * pcth + b.blue() * pcthb) / 100);
}
*/

void MediaSelectorItem::paintCell( QPainter *p, const QColorGroup &cg,
                               int column, int width, int align )
{
    QColorGroup mycg(cg);

    if ((itemPos() / height()) & 1) {
	QBrush sb = ((MediaSelector*)listView())->altBrush();
	mycg.setBrush(QColorGroup::Base, sb);
    }

    if( selectable ) {
        QCheckListItem::paintCell(p, mycg, column, width, align);
    } else {
        QListViewItem::paintCell( p, mycg, column, width, align );
    }
}


QBrush MediaSelector::altBrush() const
{
    return stripebrush;
}


void MediaSelector::paletteChange( const QPalette &p )
{
    //stripebrush = colorGroup().brush(QColorGroup::Background);
    //stripebrush = mixColors(colorGroup().base(), colorGroup().highlight(), 90);
    stripebrush = style().extendedBrush(QStyle::AlternateBase);
    QListView::paletteChange(p);
}


class MediaSelectorData {
    public:
	ContainsCategoryInterface   *catSel;
	QComboBox		    *locSel;
	QList<QListViewItem>	    shuffledList;
	bool			    haveMoved;
	bool			    doingSelection;
	QListViewItem		    *firstItem;
	QListViewItem		    *prevItem;
	int			    firstItemPos;
	int			    prevItemPos;
        bool                        selectable;
};


MediaSelector::MediaSelector( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    d = new MediaSelectorData;

    paletteChange(palette());

    d->catSel = 0;
    d->locSel = 0;
    d->haveMoved = true;
    
    d->selectable = true;

    QPEApplication::setStylusOperation( viewport(), QPEApplication::RightOnHold );

    setAllColumnsShowFocus( TRUE );
    setFrameStyle( NoFrame );

    addColumn( tr( "Selected" ) );
    addColumn( tr( "Title" ) );
    header()->hide();

    setColumnAlignment( 0, Qt::AlignCenter );
    setColumnAlignment( 1, Qt::AlignLeft );
    setColumnWidthMode( 0, QListView::Manual );
    setColumnWidthMode( 1, QListView::Manual );
    setColumnWidth( 0, CHECK_BOX_WIDTH );
    setColumnWidth( 1, 105 );

    connect( this, SIGNAL( rightButtonPressed(QListViewItem*,const QPoint&,int) ), this, SLOT( showItemProperties(QListViewItem*) ) );

    clear();

    setSorting( -1 );

    sbVis = FALSE;
}


MediaSelector::~MediaSelector()
{
    delete d;
}


MediaSelectorItem *MediaSelector::newItem( const DocLnk& lnk )
{
    MediaSelectorItem *item = new MediaSelectorItem( this, lnk );
    if( !d->selectable ) {
        item->setSelectable( false );
    }
    unsigned int randomPos = (unsigned int)((double)rand() * d->shuffledList.count() / RAND_MAX);
    randomPos = QMIN( randomPos, d->shuffledList.count() );
    d->shuffledList.insert( randomPos, item );
    if ( verticalScrollBar()->isVisible() != sbVis )
	viewportResizeEvent( 0 );
    return item;
}


void MediaSelector::deleteItem( MediaSelectorItem *item )
{
    d->shuffledList.remove( item );
    delete item;
}


// Maximize the 1st column to fill the space left from the next two columns
// Attempt to resize so no horizontal scroll bar is needed. Perhaps need to catch
// other changes like font size changes or style changes to really do this properly,
// but at the very least try to get the desired columns on the screen.
void MediaSelector::viewportResizeEvent( QResizeEvent * )
{
    QScrollBar *sb = verticalScrollBar();
    sbVis = sb->isVisible();
    int sbWidth = sbVis ? sb->width() + 1 : 0;
    if( d->selectable ) {
        setColumnWidth( 1, width() - sbWidth - 4 * lineWidth() - CHECK_BOX_WIDTH );
    } else {
        setColumnWidth( 1, width() - sbWidth - 4 * lineWidth() );
    }
}


void MediaSelector::showItemProperties( QListViewItem *i )
{
    d->haveMoved = true;
    MediaSelectorItem *item = (MediaSelectorItem *)i;
    if ( item ) {
	MediaFileInfoWidget *infoWidget = new MediaFileInfoWidget( item->file(), this );
	QPEApplication::execDialog( infoWidget );
        delete infoWidget;
    }
}

void MediaSelector::setCategoryFilter( ContainsCategoryInterface *c )
{
    d->catSel = c;
}


void MediaSelector::setLocationFilter( QComboBox *l )
{
    d->locSel = l;
}


bool MediaSelector::filtersMatch( const DocLnk& lnk )
{
    if ( d->catSel && !d->catSel->containsCategoryInterface( lnk.categories() ) )
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


void MediaSelector::removeLink( const DocLnk &lnk )
{
    QListViewItem *prevItem = 0;
    QListViewItem *item = firstChild();
    while ( item ) {
	MediaSelectorItem *i = (MediaSelectorItem *)item;
	DocLnk l = i->file();
	if ( l.name() == lnk.name() && l.file() == lnk.file() ) {
	    deleteItem( i );
	    return;
	}
	prevItem = item;
	item = item->nextSibling();
    }
}


void MediaSelector::changeLink( const DocLnk &oldDoc, const DocLnk &newDoc )
{
    QListViewItem *item = firstChild();
    while ( item ) {
	MediaSelectorItem *i = (MediaSelectorItem *)item;
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


void MediaSelector::addLink( const DocLnk &lnk )
{
    // Ignore if it is not in the category
    if ( !filtersMatch( lnk ) )
	return;
    newItem( lnk );
    // Set selected and current to first in list while adding links
    QListViewItem *first = firstChild();
    if ( first ) {
	setCurrentItem( first );
	setSelected( first, TRUE );
    }
}


void MediaSelector::removeAll()
{
    clear();
}


void MediaSelector::contentsMouseMoveEvent( QMouseEvent *event )
{
    if ( event->state() == QMouseEvent::LeftButton && !d->haveMoved ) {
	int x, y;
	contentsToViewport(event->pos().x(), event->pos().y(), x, y);
	if ( !d->doingSelection ) {
	    QListViewItem *currentItem = selectedItem();
	    QListViewItem *itemUnder = itemAt(QPoint(x, y));
	    if ( currentItem && currentItem->itemAbove() == itemUnder ) {
		d->haveMoved = true;
		moveSelectedUp();
	    } else if ( currentItem && currentItem->itemBelow() == itemUnder ) {
		d->haveMoved = true;
		moveSelectedDown();
	    }
	} else {
	    QListViewItem *currentItem = itemAt(QPoint(x, y));
	    if ( currentItem == d->prevItem )
		return;
	    int currentItemPos = currentItem->itemPos();

	    QCheckListItem *item =
		(currentItemPos == d->firstItemPos) ? (QCheckListItem *)d->prevItem :
		((currentItemPos > d->firstItemPos) == (currentItemPos > d->prevItemPos)) ?
			    (QCheckListItem *)currentItem : (QCheckListItem *)d->prevItem;
	    
	    if ( item != d->firstItem )
		item->setOn( !item->isOn() );

	    d->prevItem = currentItem;
	    d->prevItemPos = currentItemPos;
	}
    }
}


void MediaSelector::contentsMousePressEvent( QMouseEvent *event )
{
    d->haveMoved = false;
    d->doingSelection = (event->pos().x() <= CHECK_BOX_WIDTH);
    if ( d->doingSelection ) {
	int x, y;
	contentsToViewport(event->pos().x(), event->pos().y(), x, y);
	d->firstItem = itemAt(QPoint(x, y));
	if ( d->firstItem ) {
	    d->firstItemPos = d->firstItem->itemPos();
	    d->prevItem = d->firstItem;
	    d->prevItemPos = d->firstItemPos;
	}
    }
    QListView::contentsMousePressEvent(event);
}


void MediaSelector::contentsMouseReleaseEvent( QMouseEvent *event )
{
    if ( !d->doingSelection ) {
	if ( event->button() & QMouseEvent::RightButton )
	    d->haveMoved = true;

	if ( !d->haveMoved )
	    if ( event->button() & QMouseEvent::LeftButton ) {
		int x, y;
		contentsToViewport(event->pos().x(), event->pos().y(), x, y);
		if ( itemAt(QPoint(x, y)) )
		    mediaPlayerState->setPlaying();
		d->haveMoved = true;
	    }
    }
    QListView::contentsMouseReleaseEvent(event);
}


const DocLnk *MediaSelector::current() 
{
    MediaSelectorItem *item = (MediaSelectorItem *)selectedItem();
    if ( item )
	return &(item->file());
    if ( firstChild() ) { // try harder
	setSelected( firstChild(), TRUE );
	item = (MediaSelectorItem *)firstChild();
	if ( item )
	    return &(item->file());
    }
    return NULL;
}


void MediaSelector::setSelectable( bool b )
{
    d->selectable = b;
    
    if( b ) {
        setColumnWidth( 0, CHECK_BOX_WIDTH );
    } else {
        // Collapse selection column
        setColumnWidth( 0, 0 );
    }
    // ### TODO update all items with selectable
}


void MediaSelector::categorize()
{
    CategorySelectDialog *catDlg = new CategorySelectDialog("Document View", this, 0, true);
    catDlg->setAllCategories(false);
    catDlg->setMinimumWidth(180);
    QPEApplication::execDialog(catDlg);
    addSelectionToCategory(catDlg->currentCategory());
}


void MediaSelector::addSelectionToCategory(int catId)
{
    MediaSelectorItem *item = (MediaSelectorItem *)firstChild();
    while ( item ) {
        if ( item->isOn() ) {
	    QArray<int> cats = item->file().categories();
	    if ( !cats.contains(catId) ) {
		cats.resize(cats.size()+1);
		cats[cats.size()-1] = catId;
		DocLnk lnk = item->file();
		lnk.setCategories(cats);
		lnk.writeLink();
		item->setFile(lnk);
	    }
	}
	item = (MediaSelectorItem *)item->nextSibling();
    }
}


void MediaSelector::selectAll()
{
    MediaSelectorItem *item = (MediaSelectorItem *)firstChild();
    while ( item ) {
        item->setOn(true);
	item = (MediaSelectorItem *)item->nextSibling();
    }
}


void MediaSelector::addToSelector( const DocLnk &lnk )
{
    MediaSelectorItem *item = newItem( lnk );
    QListViewItem *current = selectedItem();
    if ( current )
        item->moveItem( current );
    setSelected( item, TRUE );
    ensureItemVisible( selectedItem() );
}


void MediaSelector::removeSelected()
{
    QListViewItem *item = selectedItem();
    if ( item )
	deleteItem( (MediaSelectorItem *)item );
    setSelected( currentItem(), TRUE );
    ensureItemVisible( selectedItem() );
}


void MediaSelector::moveSelectedUp()
{
    QListViewItem *item = selectedItem();
    if ( item && item->itemAbove() )
	item->itemAbove()->moveItem( item );
    ensureItemVisible( selectedItem() );
}


void MediaSelector::moveSelectedDown()
{
    QListViewItem *item = selectedItem();
    if ( item && item->itemBelow() )
        item->moveItem( item->itemBelow() );
    ensureItemVisible( selectedItem() );
}


bool MediaSelector::prev()
{
#ifdef QTOPIA_PHONE
    if ( mediaPlayerState->looping() ) 
	return TRUE;
#endif
    QListViewItem *curItem = selectedItem();
    QListViewItem *prevItem = 0;
    if ( mediaPlayerState->shuffled() ) {
	d->shuffledList.find( curItem );
	prevItem = d->shuffledList.prev();
    } else if ( curItem )
	prevItem = curItem->itemAbove();
    if ( !prevItem ) {
#ifndef QTOPIA_PHONE
	if ( mediaPlayerState->looping() ) {
	    if ( mediaPlayerState->shuffled() ) {
		prevItem = d->shuffledList.last();
	    } else {
		return last();
	    }
	} else
#endif
	    return FALSE;
    }
    setSelected( prevItem, TRUE );
    ensureItemVisible( prevItem );
    return TRUE;
}


bool MediaSelector::next()
{
#ifdef QTOPIA_PHONE
/*
    if ( mainDocumentWindow->isRinging() ) {
	mainDocumentWindow->songFinished();
	return FALSE;
    }
*/
    if ( mediaPlayerState->looping() ) 
	return TRUE;
#endif
    QListViewItem *curItem = selectedItem();
    QListViewItem *nextItem = 0;
    if ( mediaPlayerState->shuffled() ) {
	d->shuffledList.find( curItem );
	nextItem = d->shuffledList.next();
    } else if ( curItem )
	nextItem = curItem->itemBelow();
    if ( !nextItem ) {
#ifndef QTOPIA_PHONE
	if ( mediaPlayerState->looping() ) {
	    if ( mediaPlayerState->shuffled() ) {
		nextItem = d->shuffledList.first();
	    } else {
		return first();
	    }
	} else
#endif
	    return FALSE;
    }
    setSelected( nextItem, TRUE );
    ensureItemVisible( nextItem );
    return TRUE;
}


bool MediaSelector::advanceSelector()
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


bool MediaSelector::first()
{
    QListViewItem *item = firstChild();
    if ( item )
        setSelected( item, TRUE );
    else
	return FALSE;
    ensureItemVisible( selectedItem() );
    return TRUE;
}


bool MediaSelector::last()
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


void MediaSelector::keyPressEvent(QKeyEvent *ke)
{
    QCheckListItem *currentItem;
    switch ( ke->key() ) {
        case Key_Left:
        case Key_Right:
	    currentItem = (QCheckListItem *)selectedItem();
	    if ( currentItem )
		currentItem->setOn(!currentItem->isOn());
	    break;
#ifdef QTOPIA_PHONE
        case Key_Select:
#endif
        case Key_Space:
        case Key_Enter:
            mediaPlayerState->setPlaying(false);
            mediaPlayerState->setPlaying(true);
            return;
        default:
            QListView::keyPressEvent(ke);
            break;
    }
}

