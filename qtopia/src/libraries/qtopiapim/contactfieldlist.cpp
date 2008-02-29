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

#include "contactfieldlist_p.h"

#include <qevent.h>
#include <qaction.h>
#include <qsimplerichtext.h>
#include <qlayout.h>
#include "abtable_p.h"
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qpixmapcache.h>

#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/image.h>
#include <qpainter.h>


const int spc = 1;
ContactFieldListItem::ContactFieldListItem( const QPixmap &contactPix, 
			    const QString &t, ContactFieldListItem::FieldType fieldType, 
			    QListBox *lb, QListBoxItem *after, int catId, bool simContact )
    : QListBoxItem( lb, after )
{
    init();

    setContactPix( contactPix );
    setText( t );
    setFieldType( fieldType );
    mCategoryId = catId;
    mSimContact = simContact;
}

ContactFieldListItem::ContactFieldListItem( QListBox *lb )
    : QListBoxItem( lb )
{
    init();
}

ContactFieldListItem::ContactFieldListItem( QListBox *lb, QListBoxItem *after )
   : QListBoxItem( lb, after ) 
{
    init();
}

void ContactFieldListItem::init()
{
    mHighlightIndex = -1;
    mHighlightLength = 0;
    mHighlight = FALSE;

    mFieldType = ContactFieldListItem::None;
    setCustomHighlighting( TRUE );
}

void ContactFieldListItem::setHighlight( int index, int len )
{
    mHighlight = TRUE;
    mHighlightIndex = index;
    mHighlightLength = len;
}

void ContactFieldListItem::setContactPix( const QPixmap &contactPix )
{
    mContactPix = contactPix;
}

void ContactFieldListItem::setFieldType( ContactFieldListItem::FieldType type )
{
    mFieldType = type;
    ContactFieldList *lb = (ContactFieldList *)listBox();
    mFieldTypePix = lb->fieldTypePixmap( type );
}

ContactFieldListItem::FieldType ContactFieldListItem::fieldType() const
{
    return mFieldType;
}

int ContactFieldListItem::categoryId() const
{
    return mCategoryId;
}

int ContactFieldListItem::width( const QListBox *lb ) const
{
    return ( lb ? lb->visibleWidth() : 0 );
#if 0
    const int tw = ( lb ? lb->fontMetrics().boundingRect(text()).width() : 0 );
    return QMAX( mContactPix.width()+spc+tw+spc+mFieldTypePix.width(),
					QApplication::globalStrut().width() );
#endif
}

int ContactFieldListItem::height( const QListBox *lb ) const
{
    //work out the height of the line breaked text
    QFontMetrics fm = lb->fontMetrics();
#if 0
    const int cw = width( lb );
    const int mh = mContactPix.height() + (mContactPix.height() / 2);
    const int tw = cw-mFieldTypePix.width()-mContactPix.width()-(spc*2);
    //what's with this function? seems to return bogus results. 
    //doesn't take into account real character heights among other things.
    // Yes and yes are not the same size
    QRect br = fm.boundingRect( 0, 0, tw, mh, 
					Qt::WordBreak, text() );
    int h = QMAX(mContactPix.height()+spc, br.height()+spc);
#endif

#if 0
    int th = (type() == ContactFieldListItem::Contact ? fm.lineSpacing()*2 
							: fm.lineSpacing() );
#endif

    //screw it, just make everything 2 lines high
    int th = fm.lineSpacing()*2+1;
    int h = QMAX(mContactPix.height()+spc, th );
    return h;
}

SelectedContact ContactFieldListItem::id() const
{
    ContactFieldList *lb = (ContactFieldList *)listBox();
    
    int idx = lb->index( this );
    const PimContact *cnt = 0;
    if( lb->mIndexToContact.contains( idx ) )
        cnt = lb->mIndexToContact[idx];
    return SelectedContact( text(), fieldType(), lb->extraInfoText( lb->index( this ) ),
        cnt, mSimContact );
}

void ContactFieldListItem::paint( QPainter *p )
{
    int x = 0;
    int y = spc;

    ContactFieldList *lb = (ContactFieldList *)listBox();
    const int cw = width(lb), ch = height(lb);

    //backgroundDebug: F: '*EACS: 2,0,65535'

    int idx = lb->index( this );
    QBrush bg;
    QPen pen = p->pen();
    bool isSel = selected();
    QColorGroup cg = lb->colorGroup();
    const PimContact* cnt = 0;
    if( lb->mIndexToContact.contains( idx ) )
        lb->mIndexToContact[idx];

    if( isSel )
    {
	bg = lb->colorGroup().brush( QColorGroup::Highlight );
	pen.setColor( lb->colorGroup().highlightedText() );
	cg.setColor( QColorGroup::Text, cg.highlightedText() );
    }
    else
    {
	pen.setColor( lb->colorGroup().text() );
	if( idx % 2 )
	    bg = ((ContactFieldList *)lb)->mStripeBrush;
	else
	    bg = lb->colorGroup().brush( QColorGroup::Base );
    }
    p->setPen( pen );
    p->fillRect( 0, 0, cw, ch, bg );

    //in multi mode, paint check box
    if( lb->allowMultiple() )
    {
	int cbs = lb->fontMetrics().height()-spc*2;
	y = ch/2 - cbs/2;
	p->drawRect( spc*2, y, cbs, cbs );
	
        bool isChecked;
        
        if( fieldType() != Category ) 
            isChecked = lb->mSelected.contains( id() );
        else
            isChecked = lb->mSelectedCategories.contains( categoryId() );
        
        if( isChecked )
	{
#ifndef QTOPIA_DESKTOP
	    lb->style().drawCheckMark( p, spc*2+spc, y-spc, cbs, cbs, cg, TRUE, FALSE );
#else
	    //FIXME ..  qt >= 3 
	    qWarning("Drawing of check marks not implemented for qtopia desktop.");
#endif
	}
        else if( cnt )
	{
            QArray<int> contactCategories = cnt->categories();
            int numContactCategories = contactCategories.count();
            
            for( int i = 0; i < numContactCategories; ++i ) 
            {
                if( lb->mSelectedCategories.contains( contactCategories[i] ) ) 
                {
                    p->fillRect( spc*2 + 1, y + 1, cbs - 2, cbs - 2, cg.brush(QColorGroup::Mid) );
#ifndef QTOPIA_DESKTOP
                    lb->style().drawCheckMark( p, spc*2+spc, y-spc, cbs, cbs, cg, TRUE, FALSE );
#else
                    //FIXME ..  qt >= 3 
                    qWarning("Drawing of check marks not implemented for qtopia desktop.");
#endif
                    break;
                }
            }
	}
	x = spc*4 + cbs ;
	y = spc;
    }

    //type pixmap
    p->drawPixmap( x, y, mContactPix );
    x += mContactPix.width()+spc;

    //text
    int tw = cw-mFieldTypePix.width()-x-spc;
    QFont f = lb->font();
    f.setBold( TRUE );
    p->setFont( f );
    QRect br;
    int flags = Qt::WordBreak;
    /*
    if( type() != ContactFieldListItem::Contact )
	flags |= Qt::AlignVCenter;
	*/
    //don't know if this will work - using boundingRect to determine what QSimpleRichText draws
    QString itemText = text();
    br = p->boundingRect( x, 0, tw, ch, flags, itemText, -1 );
    if( mHighlight )
    {
	QString hlText = text().mid( mHighlightIndex, mHighlightLength );
	if( !hlText.isEmpty() )
	{
	    //there's no way to know where to paint with word wrap
	    //best way to do it - QSimpleRichText with underlining?
	    QString richText = itemText;
	    //put underline tags around the highlighted text
	    richText = richText.insert( mHighlightIndex+mHighlightLength, "</u>" );
	    richText = richText.insert( mHighlightIndex, "<u>" );
	    itemText = richText;
	}
    }
    QSimpleRichText srt( itemText, f );
    srt.setWidth( tw );
    srt.draw( p, x, 0, QRegion( x, 0, tw, ch ), cg );
    /*
       Do we still have enough room to draw in some extra information?
   */
    QFont smallFont = lb->font();;
    smallFont.setPointSize( smallFont.pointSize()-2 );
    QFontMetrics fm( smallFont ); 
    if( /*(type() == ContactFieldListItem::Contact) && */
		    ((ch-br.height()) >= (fm.ascent())) )
    {

	QString ei = lb->extraInfoText( idx );

	if( !ei.isEmpty() )
	{
	    QFont origFont = lb->font();
	    QPen origPen = p->pen();
	    p->setFont( smallFont );
	    if( !isSel )
	    {
		QPen dimPen = p->pen();
		dimPen.setColor( ( (ContactFieldList *)lb)->mDimTextColour );
		p->setPen( dimPen );
	    }
	    p->drawText( x, br.height()+fm.ascent(), ei );
	    p->setFont( origFont );
	    if( !isSel )
		p->setPen( origPen );
	}
    }
    x += tw+spc;

    //field type pixmap
    y += ch/2 - mFieldTypePix.height()/2;
    p->drawPixmap( x, y, mFieldTypePix );
}

//copied from addressbook abtable -mrb
QColor mixColours( const QColor &a, const QColor &b, const int &s )
{
    const int sd = 100 - s;

    return QColor( 
		   (a.red() * s + b.red() * sd ) / 100 ,
		   (a.green() * s + b.green() * sd) / 100,
		   (a.blue() * s + b.blue() * sd) / 100
		 );
}

void ContactFieldList::paletteChange( const QPalette &pal )
{
#ifndef QTOPIA_DESKTOP
    mStripeBrush = style().extendedBrush(QStyle::AlternateBase);
#else
    mStripeBrush = mixColours( colorGroup().base(),
				colorGroup().highlight(), 90 );
#endif
    mDimTextColour = mixColours( colorGroup().color(QColorGroup::Base),
				    colorGroup().color(QColorGroup::Text), 62 );
    QListBox::paletteChange( pal );
    triggerUpdate( TRUE );
}

ContactFieldList::ContactFieldList( QWidget *parent, const char *name, WFlags fl )
    : QListBox( parent, name, fl ), contacts( ContactIO::ReadOnly ), mAllowMultiple( FALSE )
{
    contacts.setSorting( PimContact::FileAs, FALSE );
    mAllContacts = contacts.sortedContacts();
    mEmptyMessage = tr("No items.");
#ifdef QTOPIA_PHONE
    mSimContactsLoaded = 0;
    mSimContactsRequested = 0;
    mPhoneLine = 0;
#endif

    paletteChange( palette() );

    setHScrollBarMode( QScrollView::AlwaysOff );

    connect( &contacts, SIGNAL(contactsUpdated()), this, SLOT(contactsChanged()) );

    // connect for pda or touchscreen phone
    connect( this, SIGNAL(clicked(QListBoxItem*)), this, SLOT(itemClicked(QListBoxItem*)) );

    // connect for keypad phone
    connect( this, SIGNAL(selected(int)), this, SLOT(itemClicked(int)) );
    //setFrameStyle( QFrame::NoFrame );

    m_noMatches = tr("No matches.");
}

ContactFieldList::~ContactFieldList()
{
#ifdef QTOPIA_PHONE
    if(mPhoneLine)
        delete mPhoneLine;
#endif
}

#ifdef QTOPIA_PHONE
void ContactFieldList::loadSimContacts()
{
    if( !mSimContactsRequested )
    {
        mSimContactsRequested = TRUE;
        
        mPhoneLine = new PhoneLine();
    
        connect(mPhoneLine->phoneBook(), 
                SIGNAL(entries(const PhoneLine&,const QString&, const QValueList<PhoneBookEntry>&)),
                this, SLOT(phonebookChanged(const PhoneLine&,const QString&,
                const QValueList<PhoneBookEntry>&)) );
    
        mPhoneLine->phoneBook()->getEntries();
    }
    else if( mSimContactsLoaded )
        emit gotSimContacts();
}
#endif

void ContactFieldList::phonebookChanged( const PhoneLine&, const QString&,
    const QValueList<PhoneBookEntry>& 
#ifdef QTOPIA_PHONE
    list
#endif
    )
{
#ifdef QTOPIA_PHONE
    mSimContacts = AbUtil::phoneBookToContacts(list, TRUE);
    mSimContactsLoaded = mSimContactsRequested = TRUE;
    emit gotSimContacts();
#endif
}

bool ContactFieldList::isSelected( const PimContact *contact, int fieldType ) const
{
    QValueList<SelectedContact>::ConstIterator it;
    for( it = mSelected.begin(); it != mSelected.end(); ++it )
        if( (*it).mContact == contact && (*it).mFieldType == fieldType )
            return TRUE;
    
    return FALSE;
}

void ContactFieldList::viewportPaintEvent( QPaintEvent *e )
{
    QListBox::viewportPaintEvent( e );
    if( !count() )
    {
	QWidget *vp = viewport();
	QPainter p( vp );
	QFont f = p.font();
	f.setBold( TRUE );
	f.setItalic( TRUE );
	p.setFont( f );
	p.drawText( 0, 0, vp->width(), vp->height(), Qt::AlignCenter, 
		( filter().isEmpty() ? mEmptyMessage : m_noMatches ) );
    }
}

void ContactFieldList::contactsChanged()
{
    contacts.setSorting( PimContact::FileAs, FALSE );
    mAllContacts = contacts.sortedContacts();
    refresh();
}

void ContactFieldList::focusInEvent( QFocusEvent * )
{
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() ) {
	if( !isModalEditing() )
	    setModalEditing( TRUE );
    }
#endif
}

void ContactFieldList::setFilter( const QString &f )
{
    if( f != mFilter )
    {
	mFilter = f;
	refresh();
    }
}

QString ContactFieldList::filter() const
{
    return mFilter;
}

void ContactFieldList::refresh()
{
    clear();
    mIndexToContact.clear();
    QFontMetrics fm = fontMetrics();
    int th = fm.lineSpacing()*2+1;
    verticalScrollBar()->setLineStep(th);
}

QPixmap ContactFieldList::fieldTypePixmap( ContactFieldListItem::FieldType type )
{
    if( type == ContactFieldListItem::None )
	return QPixmap();

    QString pixString = "addressbook/";
    switch( type )
    {
	case ContactFieldListItem::HomePhone:
	    pixString += "homephone";
	    break;
	case ContactFieldListItem::HomeFax:
	    pixString += "homefax";
	    break;
	case ContactFieldListItem::HomeMobile:
	    pixString += "homemobile";
	    break;
	case ContactFieldListItem::BusinessPhone:
	    pixString += "businessphone";
	    break;
	case ContactFieldListItem::BusinessFax:
	    pixString += "businessfax";
	    break;
	case ContactFieldListItem::BusinessMobile:
	    pixString += "businessmobile";
	    break;
	case ContactFieldListItem::BusinessPager:
	    pixString += "businesspager";
	    break;
	case ContactFieldListItem::Email:
	    pixString += "email";
	    break;
	default:
	    qDebug("pixmap for unknown field type not yet implemented TODO");
	    return QPixmap();
    }

    QPixmap fieldTypePix;
    QPixmap *cached = QPixmapCache::find( pixString + "-cfl" );
    if( cached )
    {
	fieldTypePix = *cached;
    }
    else
    {
	fieldTypePix.convertFromImage( Image::loadScaled( Resource::findPixmap( pixString ), 24, 24 ) );
	QPixmapCache::insert( pixString + "-cfl", fieldTypePix );
    }
    return fieldTypePix;
}

QPixmap ContactFieldList::simPixmap()
{
    QPixmap *cached = QPixmapCache::find( "addressbook/sim-contact-cfl" );
    if( cached )
    {
	return *cached;
    }
    else
    {
	QPixmap gp;
	gp.convertFromImage( Image::loadPortrait( 
			    Resource::findPixmap( "addressbook/sim-contact" ), 24, 30 ) );
	QPixmapCache::insert( "addressbook/sim-contact-cfl", gp );
	return gp;
    }
}

QPixmap ContactFieldList::contactPixmap( const PimContact &cnt )
{
    // XXX : Copied from the addressbook -mrb

    //try and get the contact's pixmap if it has it, otherwise load the generic one
    QString pFileName = cnt.customField( "photofile" );
    if( pFileName.isEmpty() )
        pFileName = cnt.customField( "thumbfile" );
        
    if( !pFileName.isEmpty() )
    {
	QPixmap *cached = QPixmapCache::find( "pimcontact"+cnt.uid().toString()+"-cfl" );
	if( cached )
	{
	    return *cached;
	}
	else
	{
	    QPixmap cp;
            cp = cnt.thumbnail( AbUtil::ImageWidth, AbUtil::ImageHeight );
            if( !cp.isNull() )
            {
                QPixmapCache::insert( "pimcontact"+cnt.uid().toString()+"-cfl", cp );
                return cp;
            }
	}
    }
    /* at this point the contact doesn't have an image or loading of cached image failed */
    QPixmap *cached = QPixmapCache::find( "addressbook/generic-contact-cfl" );
    if( cached )
    {
	return *cached;
    }
    else
    {
	QPixmap gp;
	gp.convertFromImage( Image::loadPortrait( 
			    Resource::findPixmap( "addressbook/generic-contact" ), 24, 30 ) );
	QPixmapCache::insert( "addressbook/generic-contact-cfl", gp );
	return gp;
    }
}

void ContactFieldList::reset()
{
    mAllContacts = contacts.sortedContacts();
    mIndexToContact.clear();
    mSelected.clear();

    clear();

    setFilter( QString::null );
}

QString ContactFieldList::extraInfoText( int ) const
{
    return QString::null;
}

void ContactFieldList::setAllowMultiple( bool allowMultiple )
{
    mAllowMultiple = allowMultiple;
}

bool ContactFieldList::allowMultiple() const
{
    return mAllowMultiple;
}

void ContactFieldList::itemClicked( QListBoxItem *i )
{
    if( allowMultiple() ) {
        if( i ) {
            ContactFieldListItem *item = (ContactFieldListItem *)i;
            
            if( item->fieldType() == -1 ) {
                //  This is a categoryId
                if( mSelectedCategories.contains( item->categoryId() ) )
                    mSelectedCategories.remove( item->categoryId() );
                else {
                    if( !allowMultiple() ) {
                        mSelected.clear();
                        mSelectedCategories.clear();
                    }
                    mSelectedCategories += item->categoryId();
                    
                    //  Make sure none of the selected contacts are in this category.
                    QValueList<SelectedContact>::Iterator it;
                    for( it = mSelected.begin(); it != mSelected.end(); ++it ) {
                        const PimContact *cnt = (*it).mContact;
                        if( cnt && cnt->categories().contains( item->categoryId() ) ) 
                            it = mSelected.remove( it );
                    }
                }
            } else {
                // This is a field. Make sure this is not implicitly selected as part of a category.
                const PimContact *cnt = 0;
                int idx = index( item );
                if( mIndexToContact.contains( idx ) )
                    cnt = mIndexToContact[idx];
                if( cnt ) {
                    QArray<int> cats = cnt->categories();
                    QValueList<int>::ConstIterator it;
                    for( it = mSelectedCategories.begin(); it != mSelectedCategories.end(); ++it ) 
                        if( cats.contains( *it ) )
                            return;
                            
                    SelectedContact clickedId = item->id();
                    if( mSelected.contains( clickedId ) && allowMultiple() ) 
                        mSelected.remove( clickedId );
                    else {
                        if( !allowMultiple() ) {
                            mSelected.clear();
                            mSelectedCategories.clear();
                        }
                        mSelected += clickedId;
                    }
                }
            }
            
            emit selectionChanged();
            triggerUpdate( FALSE );
        }
    }
}

void ContactFieldList::itemClicked( int idx )
{
    if( idx > -1 )
        itemClicked(item( idx ));
}

void ContactFieldList::setSelectionMode( QListBox::SelectionMode m )
{
    //always set single selection mode on the list box
    QListBox::setSelectionMode( QListBox::Single );
    if( m == QListBox::Single )
	setAllowMultiple( FALSE );
    else if( m == QListBox::Multi || m == QListBox::Extended )
	setAllowMultiple( TRUE );
    else
	qWarning("ContactFieldlist::setSelectionMode - NoSelection mode not supported.");
}

QValueList<int> ContactFieldList::selected() const
{
    if( filter().length() && allowMultiple() )
	qDebug("BUG : ContactFieldList::selected() called on a filtered list that allows multiple selection.");

    QValueList<int> selectedIndices;
    QValueList<SelectedContact> selectedIds = mSelected;
    for( uint i = 0 ; i < count() ; ++i )
    {
	ContactFieldListItem *ci = (ContactFieldListItem *)item( i );
	for( uint j = 0 ; j < selectedIds.count() ; ++j )
	{
	    if( selectedIds[j] == ci->id() )
	    {
		selectedIndices += i;
		selectedIds.remove( selectedIds.at( j ) );
		break;
	    }
	}
	if( !selectedIds.count() )
	    break;
    }
    return selectedIndices;
}

PimContact ContactFieldList::contactAt( int idx ) const
{
    // int(count()) as count will never be past end of ints
    if( idx >= 0 && idx < int(count()) && mIndexToContact.contains( idx ) )
	return PimContact( *(mIndexToContact[idx] ) );
    else
	return PimContact();
}

int ContactFieldList::fieldTypeAt( int idx ) const
{
    // int(count()) as count will never be past end of ints
    if( idx < 0 || idx >= int(count()) )
	return -1;
    return (int)((ContactFieldListItem *)item(idx))->fieldType();
}

QString ContactFieldList::fieldDataAt( int idx ) const
{
    int ft = fieldTypeAt( idx );
    if( ft == -1 || !mIndexToContact.contains( idx ) )
	return QString::null;
    return mIndexToContact[idx]->field( ft );
}

void ContactFieldList::setSelectedFields( const QStringList &fieldData )
{
    if( filter().length() )
    {
	qWarning("ContactFieldList::setSelectedFields called on filtered list - reseting filter.");
	setFilter( QString::null );
    }

    mSelected.clear();
    QStringList fd = fieldData;
    if( !allowMultiple() && fd.count() > 1 )
    {
	qWarning("ContactFieldList::setSelectedFields - Passed multiple fields in single selection mode. Only using first field.");
	QString tmp = fd.first();
	fd.clear();
	fd += tmp;
    }

    for( uint i = 0 ; i < count() ; ++i )
    {
	QString itemFieldData = fieldDataAt( i );
	ContactFieldListItem *ci = (ContactFieldListItem *)item( i );
	for( uint j = 0 ; j < fd.count() ; ++j )
	{
	    if( itemFieldData == fd[j] )
	    {
		mSelected += ci->id();
		QStringList::Iterator rit = fd.at( j );
		fd.remove( rit );
		if( !allowMultiple() )
		    //physically select the list box item in single mode
		    setSelected( i, TRUE );
		break;
	    }
	}
	if( !fd.count() )
	    break;
    }
    //for everything that's remaining in fd, couldn't find matching items in the list
    //just store it and make it available to the caller
    mUnknownFieldData = fd;
}

QStringList ContactFieldList::unknownFieldData() const
{
    return mUnknownFieldData;
}

QString ContactFieldList::emptyMessage()
{
    return mEmptyMessage;
}

void ContactFieldList::setEmptyMessage(QString msg)
{
    mEmptyMessage = msg;
    repaint();
}
