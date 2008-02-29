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

#include "emaildialogphone.h"

#include <qaction.h>
#include <qlayout.h>

#include <qtopia/resource.h>
#include <qtopia/contextmenu.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>

EmailDialog::EmailDialog( QWidget *parent, const char *name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    QVBoxLayout *l = new QVBoxLayout( this );
    mList = new EmailDialogList( this );
    mList->setFocusPolicy( NoFocus );
    mList->setFrameStyle( QFrame::NoFrame );

    mEdit = new EmailLineEdit( this );

    connect( mList, SIGNAL(highlighted(QListBoxItem*)), mEdit, SLOT(currentChanged(QListBoxItem*)) );
    connect( mEdit, SIGNAL(textChanged(const QString&)), mList, SLOT(setCurrentText(const QString&)) );

    connect( mEdit, SIGNAL(newEmail()), mList, SLOT(newEmail()) );
    connect( mEdit, SIGNAL(deleteEmail()), mList, SLOT(deleteEmail()) );
    connect( mEdit, SIGNAL(setAsDefault()), mList, SLOT(setAsDefault()) );
    connect( mEdit, SIGNAL(moveUp()), mList, SLOT(moveUp()) );
    connect( mEdit, SIGNAL(moveDown()), mList, SLOT(moveDown()) );

    l->addWidget( mList );
    l->addWidget( mEdit );
    mEdit->setFocus();

    setGeometry( qApp->desktop()->geometry() );

    setCaption( tr("Email List") );
}

EmailDialog::~EmailDialog()
{
}

void EmailDialog::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    mList->triggerUpdate( TRUE );
}

void EmailDialog::setEmails( const QString &def, const QStringList &em )
{
    mList->setEmails( def, em );
}

QString EmailDialog::defaultEmail() const
{
    return mList->defaultEmail();
}

QStringList EmailDialog::emails() const
{
    return mList->emails();
}

EmailDialogListItem::EmailDialogListItem( EmailDialogList *parent )
    : QListBoxItem( parent )
{
}

EmailDialogListItem::EmailDialogListItem( EmailDialogList *parent, EmailDialogListItem *after )
    : QListBoxItem( parent, after )
{
}

void EmailDialogListItem::setText( const QString &txt )
{
    QListBoxItem::setText( txt );
    listBox()->triggerUpdate( TRUE );
}

void EmailDialogListItem::setPixmap( const QPixmap &pix )
{
    mDefaultPix = pix;
    listBox()->triggerUpdate( TRUE );
}

const QPixmap *EmailDialogListItem::pixmap() const
{
    return &mDefaultPix;
}

int EmailDialogListItem::width( const QListBox *lb ) const
{
    return lb->visibleWidth();
}

const int mgn = 2;
int EmailDialogListItem::height( const QListBox *lb ) const
{
    QFontMetrics fm = lb->fontMetrics();
    const int pixWidth = (mDefaultPix.width() > 0 ? mDefaultPix.width() : 24);
    int h = QMAX( mDefaultPix.height()+mgn*2, 
					fm.boundingRect( 0, 0, width(lb)-pixWidth-mgn, 
					fm.lineSpacing(), Qt::WordBreak, text().simplifyWhiteSpace()).height()+(mgn*2) );
    return h;
}

void EmailDialogListItem::paint( QPainter *p )
{

    //TODO : width of pixmap shouldn't be a hardcoded value
    const int itemWidth = width( listBox() );
    const int itemHeight = height( listBox() );
    const int w = itemWidth;
    const int h = itemHeight-(mgn*2);
    const int pixWidth = (mDefaultPix.width() > 0 ? mDefaultPix.width() : 24);
    int x = 0,
	y = 0+mgn;
    p->drawText( x, y, w-pixWidth-mgn, h, Qt::WordBreak, text() );
    if( !mDefaultPix.isNull() )
    {
	x = w-pixWidth;
	y = itemHeight/2 - mDefaultPix.height()/2;
	p->drawPixmap( x, y, mDefaultPix );
    }
}

EmailDialogList::EmailDialogList( QWidget *parent, const char *name, WFlags fl )
    : QListBox( parent, name, fl )
{
    mDefaultPix = Resource::loadIconSet( "email" ).pixmap( QIconSet::Small, TRUE );
    mDefaultIndex = -1;
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() )
	setModalEditing( FALSE );
#endif
    setHScrollBarMode( AlwaysOff );
}

void EmailDialogList::setCurrentText( const QString &t )
{
    if( !count() && !t.isEmpty() )
    {
	newEmail( t ); // don't have items and have new text
    }
    else if( currentItem() == -1 && count() )
    {
	setCurrentItem( 0 ); //have items but no current
    }
    else if( !count() )
    {
	//have no items, and t is empty
	return;
    }

    if( item( currentItem() )->text() != t )
    {
	((EmailDialogListItem *)item( currentItem() ))->setText( t );
    }
}

void EmailDialogList::setEmails( const QString &def, const QStringList &em )
{
    clear();
    QStringList::ConstIterator it;
    mDefaultIndex = -1;
    int idxCount = 0; 
    EmailDialogListItem *prevItem = 0;
    for( it = em.begin() ; it != em.end() ; ++idxCount, ++it )
    {
	QString emTxt = (*it).simplifyWhiteSpace() ;
	if( emTxt.isEmpty() )
	    continue;

	EmailDialogListItem *newItem = new EmailDialogListItem( this, prevItem );
	newItem->setText( emTxt );
	if( emTxt == def && mDefaultIndex == -1 )
	{
	    newItem->setPixmap( mDefaultPix );
	    mDefaultIndex = idxCount;
	}
	prevItem = newItem;
    }
    if( count() )
    {
	setCurrentItem( 0 );
	ensureCurrentVisible();
    }
}

QString EmailDialogList::defaultEmail() const
{
    if( mDefaultIndex != -1 )
	return item( mDefaultIndex )->text();
    return QString::null;
}

QStringList EmailDialogList::emails() const
{
    QStringList em;
    for( uint i = 0 ; i < count() ; ++i )
    {
	if( !item( i )->text().stripWhiteSpace().isEmpty() )
	    em += item( i )->text();
    }
    return em;
}

void EmailDialogList::newEmail()
{
    newEmail( QString::null );
}

void EmailDialogList::newEmail( const QString &email )
{
    int lastIdx = count()-1;
    EmailDialogListItem *newItem = new EmailDialogListItem( this, (EmailDialogListItem *)item(lastIdx) );
    newItem->setText( (!email.isNull() ? email : tr("user@domain")) );
    setCurrentItem( lastIdx + 1 );
    ensureCurrentVisible();
    if( lastIdx == -1 )
	setAsDefault();
}

void EmailDialogList::deleteEmail()
{
    const int ci = currentItem();
    if( ci != -1 )
    {
	removeItem( ci );
	if( count() )
	{
	    int ni = (ci > 0 ? ci-1 : 0);
	    setSelected( ni, TRUE );
	    if( ci == mDefaultIndex )
	    {
		mDefaultIndex = -1;
		setAsDefault();
	    }
	}
	else
	{
	    mDefaultIndex = -1;
	}
    }
}

void EmailDialogList::setAsDefault()
{
    if( currentItem() != -1 )
    {
	if( mDefaultIndex != -1 )
	    ((EmailDialogListItem *)item( mDefaultIndex ))->setPixmap( QPixmap() );
	mDefaultIndex = currentItem();
	((EmailDialogListItem *)item( mDefaultIndex ))->setPixmap( mDefaultPix );
    }
}


void EmailDialogList::moveUp()
{
    if( !count() )
	return;

    int curIdx = currentItem();
    --curIdx;
    if( curIdx < 0 )
	curIdx = count()-1;
	setCurrentItem( curIdx );
}

void EmailDialogList::moveDown()
{
    if( !count() )
	return;
    int curIdx = currentItem();
    ++curIdx;
    if( curIdx >= (int) count() )
	curIdx = 0;
    setCurrentItem( curIdx );
}

EmailLineEdit::EmailLineEdit( QWidget *parent, const char *name )
    : QLineEdit( parent, name )
{
    ContextMenu *menu = new ContextMenu( this, 0 );

    mNewAction = new QAction( tr("New"), Resource::loadIconSet("new"), QString::null, 0, this, 0 );
    connect( mNewAction, SIGNAL(activated()), this, SIGNAL(newEmail()) );
    mNewAction->addTo( menu );

    mSetDefaultAction = new QAction( tr("Set as default"), Resource::loadIconSet("email"), QString::null, 0, this, 0 );
    connect( mSetDefaultAction, SIGNAL(activated()), this, SIGNAL(setAsDefault()) );
    mSetDefaultAction->addTo( menu );

    mDeleteAction = new QAction( tr("Delete"), Resource::loadIconSet("trash"), QString::null, 0, this, 0 );
    connect( mDeleteAction, SIGNAL(activated()), this, SIGNAL(deleteEmail()) );
    mDeleteAction->addTo( menu );
}

void EmailLineEdit::currentChanged( QListBoxItem *current )
{
    bool haveCurrent  = ( current ? TRUE : FALSE );
    QString txt = ( current ? current->text() : QString::null );
    setText( txt );
    if( current && current->listBox()->count() > 1 )
	selectAll();
    mSetDefaultAction->setEnabled( haveCurrent );
    mDeleteAction->setEnabled( haveCurrent );
}

void EmailLineEdit::keyPressEvent( QKeyEvent *ke )
{
    if( ke->key() == Key_Up )
    {
	emit moveUp();
	ke->accept();
    }
    else if( ke->key() == Key_Down )
    {
	emit moveDown();
	ke->accept();
    }
    else
    {
	QLineEdit::keyPressEvent( ke );
    }

}
