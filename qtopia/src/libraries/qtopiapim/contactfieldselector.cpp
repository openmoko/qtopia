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

#include "contactfieldselector_p.h"
#ifdef QTOPIA_PHONE
#include <qtopia/contextbar.h>
#include <qtopia/phone/phone.h>
#endif
#include <qtopia/global.h>

#include <qevent.h>
#include <qlineedit.h>
#include <qtopia/resource.h>

#include <qlayout.h>

#ifndef QTOPIA_DESKTOP
#include <qtopia/qpeapplication.h>
#endif


ContactFieldSelectorList::ContactFieldSelectorList( QWidget *parent, 
						const char *name, WFlags fl )
    : ContactFieldList( parent, name, fl )
{
#ifndef QTOPIA_DESKTOP
    setGeometry( qApp->desktop()->geometry() );
#endif
}

void ContactFieldSelectorList::setFilterFields( const QValueList<PimContact::ContactFields> &ff )
{
    mFilterFields = ff;
    refresh();
}

QValueList<PimContact::ContactFields> ContactFieldSelectorList::filterFields() const
{
    return mFilterFields;
}

void ContactFieldSelectorList::refresh()
{
    mIndexToExtraInfo.clear();

    ContactFieldList::refresh();
    const uint numContacts = mAllContacts.count();
    ContactFieldListItem *prevItem = 0;
    for( uint i = 0 ; i < numContacts ; ++i )
    {
	const PimContact *cnt = mAllContacts.at( i );
	if( !cnt )
	    continue;

	// TODO : one filter field type could represent a group of contact fields?

	//if it doesn't have atleast one of the filter fields, skip
	uint numEmptyFields = 0;
	QValueList<PimContact::ContactFields>::Iterator rit;
	for( rit = mFilterFields.begin() ; rit != mFilterFields.end() ; ++rit, ++numEmptyFields )
	    if( !(cnt->field( (int)(*rit) ).stripWhiteSpace().isEmpty()) )
		break;
	if( numEmptyFields != 0 && numEmptyFields == mFilterFields.count() )
	    continue;

	QValueList<PimContact::ContactFields> matchingFields;

	//see if the name matches
	int f = 
	    AbUtil::findPrefixInName( cnt->fileAs(), filter() );
	QStringList matchingEmails;
	if( f != -1 || filter().isEmpty() )
	{
	    for( rit = mFilterFields.begin() ; rit != mFilterFields.end() ; ++rit )
		if( !(cnt->field( (int)(*rit) ).stripWhiteSpace().isEmpty()) )
		{
		    if( *rit == PimContact::Emails )
		    {
			//add ALL email addreses
			matchingEmails = QStringList::split( " " , 
				cnt->field( (int)*rit ).stripWhiteSpace() );
			for( uint i = 0 ; i < matchingEmails.count() ; ++i )
			    matchingFields.append( *rit );
		    }
		    else
		    {
			matchingFields.append( *rit );
		    }
		}
	}
	else
	{
	    //name doesn't match, check to see if any of the filter fields match
	    for( rit = mFilterFields.begin() ; rit != mFilterFields.end() ; ++rit )
	    {
		switch( *rit )
		{
		    case PimContact::HomePhone:
		    case PimContact::HomeMobile:
		    case PimContact::BusinessPhone:
		    case PimContact::BusinessMobile:
		    {
#ifdef QTOPIA_PHONE
			if( PhoneUtils::matchPrefix( cnt->field( *rit ), filter() ) )
			    matchingFields.append( *rit );
#else
			//TODO PDA
#endif
			break;
		    }
		    case PimContact::Emails:
		    {
			QStringList emails = QStringList::split( " ", cnt->field( *rit )
								.stripWhiteSpace() );
			for( QStringList::Iterator eit = emails.begin () ; eit !=
							    emails.end() ; ++eit )
			{
#ifdef QTOPIA_PHONE
			    if( AbUtil::phoneKeyMatchN( 
				(*eit).stripWhiteSpace().lower(), 0,
				filter().lower(), filter().length() )
			       == (int)filter().length() )
#else
			    if( (*eit).stripWhiteSpace().lower().startsWith( 
								filter().lower() ) )
#endif
			    {
				matchingEmails += (*eit).lower();
				matchingFields.append( *rit );
				break;
			    }
			}
			break;
		    }
		    default:
			break;
		}
	    }
	}

	if( matchingFields.count() )
	{
	    //create list items for all things that match. 
	    uint matchingEmailCount = 0;
	    for( rit = matchingFields.begin() ; rit != matchingFields.end() ; ++rit )
	    {
		ContactFieldListItem *newItem = new ContactFieldListItem( 
		    contactPixmap( *cnt ), cnt->fileAs(), 
		    (ContactFieldListItem::FieldType)*rit, this, prevItem );

		if( f != -1 )
		    newItem->setHighlight( f, filter().length() );

		int idx = index( newItem );
		mIndexToContact[idx] = cnt;
		mIndexToExtraInfo[idx] = ( *rit != PimContact::Emails  ? 
						cnt->field( (int)*rit ) : 
					matchingEmails[matchingEmailCount++] );
		prevItem = newItem;
	    }
	    setSelected( 0, TRUE );
	}
	ensureCurrentVisible();
    }
}

void ContactFieldSelectorList::reset()
{
    mFilterFields.clear();
    mIndexToExtraInfo.clear();
    ContactFieldList::reset();
}

QString ContactFieldSelectorList::extraInfoText( int idx ) const
{
    if( mIndexToExtraInfo.contains( idx ) )
	return mIndexToExtraInfo[idx];
    else
	return QString::null;
}

ContactFieldSelector::ContactFieldSelector( 
	const QValueList<PimContact::ContactFields> &ff, 
	const QString &filterStr, bool allowMultiple, QWidget *parent, const char *name,
	bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    init();

    setFilterFields( ff );
    setFilter( filterStr );
    setAllowMultiple( allowMultiple );
}

ContactFieldSelector::ContactFieldSelector( QWidget *parent, const char *name,
						bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), mFindLE( 0 )
{
    init();
}

void ContactFieldSelector::init()
{
    mFirstSetAllowMultiple = TRUE;
    QVBoxLayout *l = new QVBoxLayout( this );
    mFieldList = new ContactFieldSelectorList( this, "fieldList" );
    mFieldList->installEventFilter( this );

    if( Global::mousePreferred() ) {
	mFieldList->setFocusPolicy( NoFocus );
	mFindLE = new QLineEdit( this );
	mFindLE->installEventFilter( this );


#ifndef QTOPIA_DESKTOP
	//QPEApplication::setInputMethodHint( mFindLE, QPEApplication::Words );
#endif
	connect( mFindLE, SIGNAL(textChanged(const QString&)), mFieldList, SLOT(setFilter(const QString&)) );
	l->addWidget( mFindLE );
    }

    l->addWidget( mFieldList );
    setAllowMultiple( FALSE );
    setCaption( tr("Select Contact") );
}

bool ContactFieldSelector::eventFilter( QObject *o, QEvent *e )
{
    if( o == mFieldList && e->type() == QEvent::KeyPress )
    {
	QKeyEvent *ke = (QKeyEvent *)e;
	if( !ke->text().isEmpty() )
	{
	    setFilter( filter()+ke->text() );
#ifdef QTOPIA_PHONE
	    if( !Global::mousePreferred() ) {
		ContextBar::setLabel( mFieldList, Key_Back, ContextBar::BackSpace );
	    }
#endif
	    return TRUE;
	}
	else if( 
		ke->key() == Key_BackSpace	
	      )
	{
	    if( filter().length() )
		setFilter( filter().left( filter().length()-1 ) );
	    if( filter().isEmpty() )
	    {
#ifdef QTOPIA_PHONE
		if( !Global::mousePreferred() ) {
		    ContextBar::setLabel( mFieldList, Key_Back,  ContextBar::Back,
							ContextBar::ModalAndNonModal );
		}
#endif
	    }
	    return TRUE;
	}
#ifdef QTOPIA_PHONE
	else if( !Global::mousePreferred() && ke->key() == Key_Back 
		|| ke->key() == Key_No 
		)
	{
	    if( !filter().isEmpty() )
	    {
		setFilter( QString::null );
	    ContextBar::setLabel( mFieldList, Key_Back,  ContextBar::Back, 
						    ContextBar::ModalAndNonModal );
		return TRUE;
	    }
	}
#endif
    }
#ifdef QTOPIA_PHONE
    else if( Global::mousePreferred() && o == mFindLE && e->type() == QEvent::KeyPress )
    {
	QKeyEvent *ke = (QKeyEvent *)e;
	switch( ke->key() )
	{
	    case Key_Select:
	    case Key_Left:
	    case Key_Right:
	    case Key_Up:
	    case Key_Down:
		QPEApplication::postEvent( mFieldList, 
		new QKeyEvent( QEvent::KeyPress, ke->key(), 0xffff, 0xffff ) );
		return TRUE;
	}
    }
#endif
    return FALSE;
}

void ContactFieldSelector::setFilterFields( const QValueList<PimContact::ContactFields> &ff )
{
    mFieldList->setFilterFields( ff );
}

QValueList<PimContact::ContactFields> ContactFieldSelector::filterFields() const
{
    return mFieldList->filterFields();
}

void ContactFieldSelector::setFilter( const QString &filterStr )
{
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() )
	mFieldList->setFilter( filterStr );
    else
#endif
    mFindLE->setText( filterStr );
}

QString ContactFieldSelector::filter() const
{
    return mFieldList->filter();
}

void ContactFieldSelector::setAllowMultiple( bool am )
{
    if( !mFirstSetAllowMultiple && allowMultiple() == am ) // don't keep connecting/disconnect slots 
	return;
    mFirstSetAllowMultiple = FALSE;

    mFieldList->setAllowMultiple( am );
    if( am )
    {
	disconnect( mFieldList, SIGNAL(selected(int)), this, SLOT(accept()) );
	disconnect( mFieldList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(accept()) );
    }
    else
    {
	connect( mFieldList, SIGNAL(selected(int)), this, SLOT(accept()) );
	connect( mFieldList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(accept()) );
    }
#ifndef QTOPIA_DESKTOP
    QPEApplication::setMenuLike( this, !am );
#endif
}

bool ContactFieldSelector::allowMultiple() const
{
    return mFieldList->allowMultiple();
}

bool ContactFieldSelector::hasSelected() const
{
    if( mFieldList->allowMultiple() )
        return mFieldList->selected().count() > 0;
    else
        return ( mFieldList->currentItem() > -1 );
}

QValueList<int> ContactFieldSelector::selected() const
{
    if( mFieldList->allowMultiple() )
        return mFieldList->selected();
    else if( mFieldList->currentItem() > -1 )
    {
        QValueList<int> selList;
        selList += mFieldList->currentItem();
        return selList;
    }
    else
        return QValueList<int>();
}

PimContact ContactFieldSelector::contactAt( int idx ) const
{
    return mFieldList->contactAt( idx );
}

int ContactFieldSelector::fieldTypeAt( int idx ) const
{
    return mFieldList->fieldTypeAt( idx );
}

QString ContactFieldSelector::fieldDataAt( int idx ) const
{
    return mFieldList->fieldDataAt( idx );
}

void ContactFieldSelector::refresh()
{
    mFieldList->refresh();
}

void ContactFieldSelector::accept()
{
    if( hasSelected() )
	emit selected( this, selected() );
    QDialog::accept();
}

void ContactFieldSelector::setSelectedFields( const QStringList &fieldData )
{
    mFieldList->setSelectedFields( fieldData );
}

QStringList ContactFieldSelector::unknownFieldData() const
{
    return mFieldList->unknownFieldData();
}
