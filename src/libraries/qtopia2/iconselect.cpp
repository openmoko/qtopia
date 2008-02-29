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

#include <qlistbox.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qlist.h>
#include <qiconset.h>

#include <qtopia/qpeapplication.h>

#include "iconselect.h"

class IconSelectData
{
public:
    IconSelectData()
    {
    }
    IconSelectData( const QPixmap &p, const QString &t )
    {
	pix = p;
	text = t;
    }

    QString text;
    QPixmap pix;
};

class IconSelectPrivate
{
public:
    IconSelectPrivate()
    {
	count = 0;
	current = -1;
	data.setAutoDelete( TRUE );
	mouseWasInsidePopup = FALSE;
    }
    QListBox *list;
    QList<IconSelectData> data;
    int count;
    int current;
    bool mouseWasInsidePopup;
    QIconSet iconSet;
};

//widget like a combobox that allows, but doesn't use text when not popped up
IconSelect::IconSelect( QWidget *parent, const char *name )
    : PixmapDisplay( parent, name )
{
    init();
}

IconSelect::IconSelect( const QPixmap &pix, QWidget *parent, const char *name )
    : PixmapDisplay( pix, parent, name )
{
    init();
}

IconSelect::~IconSelect()
{
    delete d->list;
    delete d;
}

void IconSelect::init()
{
    d = new IconSelectPrivate();
    d->list = new QListBox( this, "iconSelectList", WType_Popup );
    d->list->setMouseTracking( TRUE );
    d->list->setVariableWidth( TRUE );
    d->list->setAutoScrollBar( FALSE );
    d->list->setBottomScrollBar( FALSE );
    d->list->setAutoBottomScrollBar( FALSE );
    d->list->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->list->setLineWidth( 1 );
    //d->list->resize( 100, 10 );

    connect( d->list, SIGNAL(selected(int)), this, SLOT(itemSelected(int)) );
    connect( this, SIGNAL(clicked()), this, SLOT(popup()) );
}

uint IconSelect::count()
{
    return d->data.count();
}

void IconSelect::insertItem( const QPixmap &pix, const QString &text )
{
    d->data.insert( d->count, new IconSelectData( pix, text ) );
    if( !d->count )
	setCurrentItem( d->count );
    ++d->count;
}

void IconSelect::removeItem( int index )
{
    d->data.remove( index );
    if( d->count > 0 )
	--d->count;
    setCurrentItem( d->count ? d->count : -1 );
}

void IconSelect::clear()
{
    d->data.clear();
    setCurrentItem( -1 );
}

int IconSelect::currentItem() const
{
    return d->current;
}

void IconSelect::setCurrentItem( int index )
{
    if( index >= (int)d->data.count() || index < 0 )
	return;

    d->current = index;
    if( index == -1 )
    {
	setPixmap( QPixmap() );
    }
    else
    {
	setPixmap( d->data.at( index )->pix );
	emit activated(	index );
    }
}

void IconSelect::setPixmap( const QPixmap &pix )
{
    d->iconSet.setPixmap( pix, QIconSet::Small );
    PixmapDisplay::setPixmap( d->iconSet.pixmap( QIconSet::Small, isEnabled() ) );
}

void IconSelect::enabledChange( bool enabled )
{
    PixmapDisplay::setPixmap( d->iconSet.pixmap( QIconSet::Small, enabled ) );
    PixmapDisplay::enabledChange( enabled );
}

QPixmap IconSelect::pixmap() const
{
    return d->iconSet.pixmap();
}

void IconSelect::popup()
{
    d->list->clear();
    d->mouseWasInsidePopup = FALSE;
    QListIterator<IconSelectData> it( d->data );
    for( ; *it ; ++it )
	d->list->insertItem( (*it)->pix, (*it)->text );

    //begin copy from QComboBox
    int sw = qApp->desktop()->width();                      // screen width
    int sh = qApp->desktop()->height();                     // screen height
    QPoint pos = mapToGlobal( QPoint(0,height()) );
    int x = pos.x();
    int y = pos.y();
    int totalItemsHeight = 0;
    for( unsigned int i = 0 ; i < d->list->count() ; ++i )
	totalItemsHeight += d->list->item( i )->height( d->list );
    int totalHeight = totalItemsHeight+(d->list->frameWidth()*2)+2;
    d->list->resize( d->list->maxItemWidth()+d->list->frameWidth()*2+2, totalHeight );
    int w = d->list->width();
    int h = d->list->height();
    // the complete widget must be visible
    if ( x + w > sw )
	x = sw - w;
    else if ( x < 0 )
	x = 0;
    if (y + h > sh ) 
    {
	if ( sh - y > y ) 
	{
	    d->list->resize(w, sh - y);
	} 
	else 
	{
	    if ( y - h - height() >= 0 ) 
	    {
		y = y - h - height();
	    } 
	    else 
	    {
		//d->list->resize(w, y);
		y = 0;
	    }
	}
    }
    //end copy from QComboBox
    d->list->installEventFilter( this );
    d->list->viewport()->installEventFilter( this );

    bool block = d->list->signalsBlocked();
    d->list->blockSignals( TRUE );
    d->list->setCurrentItem( currentItem() );
    d->list->blockSignals( block );

    d->list->move( x, y );
    //d->list->adjustSize();
    d->list->show();
}

void IconSelect::popdown()
{
    d->list->removeEventFilter( this );
    d->list->viewport()->removeEventFilter( this );
    d->list->hide();
}

void IconSelect::itemSelected( int index )
{
    popdown();
    setCurrentItem( index );
}

bool IconSelect::eventFilter( QObject *obj, QEvent *e ) //mostly copied from qcombobox
{
    if( !e ) return TRUE;
    if( obj == d->list || obj == d->list->viewport() )
    {
	QKeyEvent *ke = (QKeyEvent *)e;
	QMouseEvent *me = (QMouseEvent *)e;
	switch( e->type() )
	{
	    case QEvent::KeyPress:
	    {
		switch( ke->key() ) 
		{
		    case Key_Up:
		    case Key_Down:
			if ( !(ke->state() & AltButton) )
			    break;
		    case Key_F4:
		    case Key_Escape:
#ifdef QTOPIA_PHONE
		    case Key_Back:
		    case Key_No:
#endif
		    popdown();
		    return TRUE;
		    break;
#ifdef QTOPIA_PHONE
		    case Key_Select:
#endif
		    case Key_Enter:
		    case Key_Return:
		    // work around QDialog's enter handling
		    return FALSE;
		    default:
		    break;
		}
	    }
	    case QEvent::MouseMove:
	    {
		QPoint pos = me->pos();
		if ( !d->mouseWasInsidePopup ) 
		{
		    if ( d->list->rect().contains( pos ) )
			d->mouseWasInsidePopup = TRUE;
		} 
		if( d->list->rect().contains( pos ) ) 
		{
		    QListBoxItem *item = d->list->itemAt( ((QMouseEvent *)e)->pos() );
		    if( item )
			d->list->setCurrentItem( d->list->index( item ) );
		}

		break;
	    }
	    case QEvent::MouseButtonRelease:
	    {
		if( !d->list->rect().contains( me->pos() ) && d->mouseWasInsidePopup ) 
			popdown();
		else if ( d->list->rect().contains( me->pos() ) ) 
		{
		    QMouseEvent tmp( QEvent::MouseButtonDblClick,
		    me->pos(), me->button(), me->state() ) ;
		    // will hide popup
		    QApplication::sendEvent( obj, &tmp );
		    QListBoxItem *item = d->list->itemAt( me->pos() );
		    if( item )
			d->list->setSelected( item, TRUE );
		    popdown();
		}
		else {
		    return FALSE;
		}
		return TRUE;
	    }
	    case QEvent::MouseButtonPress:
	    {
		if ( !d->list->rect().contains( me->pos() ) ) 
		{
		    popdown();
		}
		else
		{
		    d->list->setCurrentItem( d->list->index( d->list->itemAt( me->pos() ) ) );
		}
		return TRUE;
	    }
	    default:
		break;
	}
    }
    return QWidget::eventFilter( obj, e );
}

void IconSelect::keyPressEvent( QKeyEvent *e )
{
    PixmapDisplay::keyPressEvent( e );
}
