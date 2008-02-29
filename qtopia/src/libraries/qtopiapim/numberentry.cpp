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

#include <qtopia/pim/private/numberentry_p.h>
#include <qtopia/qpeapplication.h>
#ifdef QTOPIA_PHONE
# include <qtopia/phone/phone.h>
# include <qtopia/phone/phonebook.h>
# include <qtopia/contextmenu.h>
# include <qtopia/contextbar.h>
#endif
#include <qtopia/pim/private/abtable_p.h>
#include <qtopia/image.h>
#include <qtopia/resource.h>
#include <qpixmapcache.h>
#include <qtopia/categorydialog.h>

NumberEntryDialog::NumberEntryDialog( QValueList<PimContact::ContactFields> homeFields,
    QValueList<PimContact::ContactFields> busFields, bool textEntry, const char *caption,
    QWidget *parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    setCaption( caption );
    mLayout = new QVBoxLayout( this );
    
    mSelectedCount = 0;
    mSelectedString = QString::null;
    mNumberEdit = new QLineEdit( this );
    if(!textEntry)
        QPEApplication::setInputMethodHint(mNumberEdit, QPEApplication::PhoneNumber);
    mLayout->addWidget( mNumberEdit );
    
    mNumberList = new NumberSelector( homeFields, busFields, this );
    mLayout->addWidget( mNumberList );
    
    connect( mNumberEdit, SIGNAL(textChanged(const QString&)), this, 
        SLOT(numberChanged(const QString&)) );
        
    connect( mNumberList, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
    
    mNumberEdit->installEventFilter( this );
    mNumberList->installEventFilter( this );
    installEventFilter( this );
    
#ifdef QTOPIA_PHONE
    mActionGroup = new QAction( tr("Add to Category"), Resource::loadIconSet("new"), 
        QString::null, 0, this, 0 );
    connect( mActionGroup, SIGNAL(activated()), this, SLOT(addToCategory()) );
    mContextMenu = new ContextMenu( this );
    mActionGroup->addTo( mContextMenu );

    ContextBar::setLabel( this, Key_Back, ContextBar::Cancel );
#endif
        
    mStatusLabel = new QLabel( tr("0 selected"), this );
    mLayout->addWidget( mStatusLabel );
    
    mNumberEdit->setFocus();
#ifdef QTOPIA_PHONE
    mNumberEdit->setModalEditing( TRUE );
#endif
}

NumberEntryDialog::~NumberEntryDialog()
{
    delete mNumberEdit;
    delete mNumberList;
    delete mLayout;
}

void NumberEntryDialog::accept()
{
    mResults = mNumberList->selectedNumbers();
    mSelectedCount = 0;
    mSelectedString = QString::null;
    QDialog::accept();
}

void NumberEntryDialog::selectionChanged()
{
    mStatusLabel->setText( tr("%1 selected", "as in '5 selected'" ).arg(mNumberList->selectedCount()) );
    int oldSelectedCount = mSelectedCount;
    QString oldSelectedString = mSelectedString;
    mSelectedCount = mNumberList->selectedCount();
    if (mNumberList->currentItem() != -1) {
	mSelectedString = mNumberList->fullText( mNumberList->currentItem() );
    } else {
	mSelectedString == QString::null;
    }

    if (mSelectedCount != oldSelectedCount) {
	if ( !mNumberEdit->text().isEmpty() ) {
	    // Clear number edit when item is checked (or unchecked)
	    mNumberEdit->clear();
	}
	
	if ( mNumberList->hasFocus() ) {
	    // Ensure newly checked (or unchecked) item is current and visible
	    int i = 0;
	    while ( (uint)i < mNumberList->count() ) {
		if ( oldSelectedString == mNumberList->fullText( i ) ) {
		    mNumberList->setCurrentItem( i );
		    mNumberList->centerCurrentItem(); 
		    break;
		}
		++i;
	    }
	}
    }
    
#ifdef QTOPIA_PHONE
    if( mNumberEdit->text().isEmpty() ) 
    {
        ContextBar::setLabel( mNumberList, Key_Back, ContextBar::Next );
        ContextBar::setLabel( this, Key_Back, ContextBar::Next );
        ContextBar::setLabel( mNumberEdit, Key_Back, ContextBar::Next );        
    }
    else 
    {
        ContextBar::setLabel( mNumberList, Key_Back, ContextBar::BackSpace );
        ContextBar::setLabel( this, Key_Back, ContextBar::BackSpace );
        ContextBar::setLabel( mNumberEdit, Key_Back, ContextBar::BackSpace );
    }
#endif
}

QStringList NumberSelector::selectedNumbers()
{
    QStringList mResults;
    
    QValueList<SelectedContact>::ConstIterator cit;   
    for( cit = mSelected.begin(); cit != mSelected.end(); ++cit )
        mResults += (*cit).mExtraInfoText; //mContact->field( (*cit).mFieldType );

    QValueList<PimContact::ContactFields> fields;
    const int numContacts = mAllContacts.count();
    for( int i = 0; i < numContacts; ++i ) {
        const PimContact* cnt = mAllContacts.at( i );
        if( cnt ) {
            QValueList<int>::ConstIterator catIt;
            for( catIt = mSelectedCategories.begin(); catIt != mSelectedCategories.end(); ++catIt) {
                if( cnt->categories().contains( *catIt ) ) {
                    //  This contact is included. Include the first required field type in my list.

                    if( cnt->customField("BUSINESS_CONTACT").isEmpty() )
                        fields = mHomeFields;
                    else
                        fields = mBusFields;

                    QValueList<PimContact::ContactFields>::ConstIterator fieldIt;
                    for( fieldIt = fields.begin(); fieldIt != fields.end(); ++fieldIt ) {
                        int field = *fieldIt;
                        if( field == PimContact::Emails )
                            field = PimContact::DefaultEmail;
                        if( !cnt->field( field ).isEmpty() ) {
                            mResults += cnt->field( field );
                            break;
                        }
                    }
                    
                    break;
                }
            }
        }
    }
        
    return mResults;
}

QList<PimContact> NumberSelector::selectedContacts()
{
    QList<PimContact> mResults;
    
    QValueList<SelectedContact>::ConstIterator cit;   
    for( cit = mSelected.begin(); cit != mSelected.end(); ++cit )
        mResults.append((*cit).mContact);

    QValueList<PimContact::ContactFields> fields;
    const int numContacts = mAllContacts.count();
    for( int i = 0; i < numContacts; ++i ) {
        const PimContact* cnt = mAllContacts.at( i );
        if( cnt ) {
            QValueList<int>::ConstIterator catIt;
            for( catIt = mSelectedCategories.begin(); catIt != mSelectedCategories.end(); ++catIt) {
                if( cnt->categories().contains( *catIt ) ) {
                    mResults.append(cnt);
                    break;
                }
            }
        }
    }
        
    return mResults;
}

void NumberEntryDialog::numberChanged(const QString& number)
{
    QString n = number;
#ifdef QTOPIA_PHONE
    //  "Fuzzy" matching based on keypad number-letter correlations
    n = PhoneUtils::resolveLetters( n );
#endif

    mNumberList->setFilter( n );
    mNumberList->refresh();
    
#ifdef QTOPIA_PHONE
    if( n.isEmpty() ) 
    {
        ContextBar::setLabel( mNumberEdit, Key_Back, ContextBar::Next );            
        ContextBar::setLabel( mNumberEdit, Key_Select, ContextBar::NoLabel );
    }
    else 
    {
        ContextBar::setLabel( mNumberEdit, Key_Back, ContextBar::BackSpace );
        ContextBar::setLabel( mNumberEdit, Key_Select, ContextBar::Select );
    }
#endif
}

bool NumberEntryDialog::eventFilter(QObject *o, QEvent *e)
{
    switch( e->type() )
    {
        case QEvent::KeyPress:
            return handleKeyPress( o, (QKeyEvent*)e );
        
        case QEvent::FocusIn:
            handleFocus( o );
            return FALSE;
            
        default:
            return FALSE;
    }
}

void NumberEntryDialog::setTextEntry( const QString& text )
{
    mNumberEdit->setText( text );
}

void NumberEntryDialog::setSelected( const QStringList& selectedList )
{
    mNumberList->setSelectedList( selectedList );
}

void NumberSelector::setSelectedList( QStringList selectedList )
{
    if( selectedList.count() == 0)
        return;    
        
    int numContacts = mAllContacts.count();

    for( int i = 0; i < numContacts; ++i )
    {
        const PimContact *cnt = mAllContacts.at( i );
        QValueList<PimContact::ContactFields>::ConstIterator fit;

        for( fit = mHomeFields.begin(); fit != mHomeFields.end(); ++fit )
        {
            if( *fit == PimContact::Emails )
            {
                QStringList emails = cnt->emailList();
                QStringList::ConstIterator eit;

                for( eit = emails.begin(); eit != emails.end(); ++eit )
                {
                    QValueList<QString>::Iterator sit = selectedList.find( *eit );
                    if( sit != selectedList.end() )
                    {
                        //  Select this entry..!
                        mSelected += SelectedContact( cnt->fileAs(), *fit, *eit, cnt, FALSE );

                        selectedList.remove( sit );
                        if( selectedList.count() == 0)
                            return;
                    }
                }
            }
            else
            {
                QStringList::Iterator sit = selectedList.find( cnt->field( *fit ) );
                if( sit != selectedList.end() )
                {
                    //  Selected this entry..!
                    mSelected += SelectedContact( cnt->fileAs(), *fit, 
                        cnt->field( *fit ), cnt, FALSE );

                    selectedList.remove( sit );
                    if( selectedList.count() == 0)
                        return;
                }
            }
        }
    }
    
    if( selectedList.count() != 0)
    {
        //  Unknown entries: Let's just add them manually.
        QStringList::ConstIterator sit;
        for( sit = selectedList.begin(); sit != selectedList.end(); ++sit )
            addUserEntry( *sit );
    }
    
    refresh();
}

QString NumberSelector::fullText( int i )
{
    
    QString res = item( i )->text() + " ";
    res += extraInfoText( i );
    return res;
}

void NumberEntryDialog::handleFocus(QObject *
#ifdef QTOPIA_PHONE
    o
#endif
    )
{
#ifdef QTOPIA_PHONE
    if( o == mNumberEdit )
    {
        mNumberList->setModalEditing( FALSE );
        if( mNumberEdit->text().isEmpty() )
        {
            ContextBar::setLabel( mNumberEdit, Key_Select, ContextBar::NoLabel );
            ContextBar::setLabel( mNumberEdit, Key_Back, ContextBar::Next);
	    mNumberEdit->setModalEditing( TRUE );
        }
        else
        {
            ContextBar::setLabel( mNumberEdit, Key_Select, ContextBar::Select );
            ContextBar::setLabel( mNumberEdit, Key_Back, ContextBar::BackSpace );
	    mNumberEdit->setModalEditing( TRUE );
        }
    }
    else if( o == mNumberList )
    {
        ContextBar::setLabel( mNumberList, Key_Select, ContextBar::Select );
    }
    else {
        mNumberEdit->setFocus();
    }
#endif
}
#include <qmessagebox.h>
bool NumberEntryDialog::handleKeyPress(QObject *
#ifdef QTOPIA_PHONE
    o
#endif
    , QKeyEvent *
#ifdef QTOPIA_PHONE
    ke
#endif
    )
{
#ifdef QTOPIA_PHONE
    int key = ke->key();
    
    switch(key)
    {
        case Qt::Key_No:
        case Qt::Key_Back:
        case Qt::Key_BackSpace:
        {
            if( o == mNumberList )
                mNumberList->setSelected( mNumberList->currentItem(), FALSE );
                            
            QString text = mNumberEdit->text();
            int textLen = text.length();
            
            if(textLen > 0) 
            {
                mNumberEdit->setText(text.left(textLen - 1));
                if( textLen == 1 )
                    mNumberEdit->setModalEditing( FALSE );
                mNumberEdit->setFocus();
            }
            else
            {
                    accept();
            }
            
            return TRUE;
        }
    }
        
    if(o == mNumberEdit)
    {
        switch(key)
        {
            case Qt::Key_Down:
            {
                if(mNumberList->count() > 0)
                {
                    mNumberList->setSelected(0, TRUE);
                    mNumberList->ensureCurrentVisible();
                    mNumberList->setFocus();
                    return TRUE;
                }
            }
            
            case Qt::Key_Up:
            {
                if(mNumberList->count() > 0)
                {
                    mNumberList->setSelected(mNumberList->count() - 1, TRUE);
                    mNumberList->ensureCurrentVisible();
                    mNumberList->setFocus();
                    return TRUE;
                }
            }
            
            case Qt::Key_Select:
            {
                QString text = mNumberEdit->text();
                if( !text.isEmpty() )
                {
                    mNumberEdit->setText("");
                    mNumberList->addUserEntry( text );
                    mNumberEdit->setModalEditing( FALSE );
		    mNumberList->setCurrentItem( mNumberList->count() - 1 );
                }
            
                return TRUE;
            }
        }
    }
    else
    {
        switch(key)
        {
            case Qt::Key_Up:
            {
                if(mNumberList->count() == 0 || mNumberList->currentItem() == 0)
                {
                    mNumberList->setSelected(mNumberList->currentItem(), FALSE);
                    mNumberEdit->setFocus();
                    return TRUE;
                }
                break;
            }
                
            case Qt::Key_Down:
            {
                if(mNumberList->count() == 0 || 
                    mNumberList->currentItem() == (int)mNumberList->count() - 1)
                {
                    mNumberList->setSelected(mNumberList->currentItem(), FALSE);
                    mNumberEdit->setFocus();
                    return TRUE;
                }
                break;
            }
            
            case Qt::Key_Select:
            {
                return FALSE;
            }
            
            default:
            {
                //  Send the key to the text field instead.
                mNumberEdit->setFocus();
                return TRUE;
            }
        }
    }
#endif

    return FALSE;
}

NumberSelector::NumberSelector(QValueList<PimContact::ContactFields> homeFields,
    QValueList<PimContact::ContactFields> busFields, QWidget *parent, const char *name)
    : ContactFieldList(parent, name)
{
    //  Go through all contacts to construct a list of categories that contain contacts,
    //  make sure it is sorted in alphabetical order.
    
    mHomeFields = homeFields + busFields;
    mBusFields = busFields + homeFields;
    
    QString catFileName = categoryFileName();
    QValueList<int>::Iterator cit;
    if( mCategoryManager.load( catFileName ) ) {
        int numContacts = mAllContacts.count();
        for( int i = 0; i < numContacts; ++i ) {
            const PimContact *contact = mAllContacts.at( i );
            if( !contact ) continue;

            //  Make sure this contact contains an interesting detail...
            bool cntHasInterestingField = FALSE;
            QValueList<PimContact::ContactFields>::Iterator fit;
            for( fit = mHomeFields.begin(); fit != mHomeFields.end(); ++fit ) {
                if( !contact->field( *fit ).isEmpty() ) {
                    cntHasInterestingField = TRUE;
                    break;
                }
            }
            
            if( !cntHasInterestingField )
                continue;
            
            QArray<int> contactCategories = contact->categories();
            int numContactCategories = contactCategories.count();
            for( int j = 0; j < numContactCategories; ++j ) {
                
                QString contactCatLabel = mCategoryManager.label("Address Book",
                    contactCategories[j]);
                bool insertBefore = FALSE;
                bool alreadyThere = FALSE;
                
                for( cit = mCategories.begin(); cit != mCategories.end(); ++cit) {
                    QString curCatLabel = mCategoryManager.label( "Address Book", *cit );
                    if( curCatLabel  < contactCatLabel ) {
                        insertBefore = TRUE;
                        break;
                    } else if( curCatLabel == contactCatLabel ) {
                        alreadyThere = TRUE;
                        break;
                    }
                }
                
                if( !alreadyThere ) {
                    if( insertBefore ) 
                        mCategories.insert( cit, contactCategories[j] );
                    else
                        mCategories.append( contactCategories[j] );
                }
            }
        }
    }
    
    mUserContacts.setAutoDelete( TRUE );
    
    setAllowMultiple( TRUE );
    
    refresh();
}

void NumberSelector::refresh()    
{
    ContactFieldList::refresh();
    
    // Work out which contacts match
    const int numContacts = mAllContacts.count();
    const QString filStr = filter();
    const int filLen = filStr.length();
    QList<MatchingContact> matchingContacts;
    matchingContacts.setAutoDelete( TRUE );
    QValueList<int>::ConstIterator catIt;
    QValueList<QUuid> matchedContacts;
    QValueList<int> matchedCategories;
    bool useSimContact = FALSE;    
#ifdef QTOPIA_PHONE
    PimContact* simCnt = mSimContacts.first();
#endif    
   
    ContactFieldListItem *prevItem = 0;

    //  Work out which contacts and categories match
    catIt = mCategories.begin();
    QString catName = mCategoryManager.label( "Address Book", *catIt );
    if( catName[0] == '_' )
        catName = catName.mid( 1 );
    
    for( int i = 0 ; i < numContacts 
#ifdef QTOPIA_PHONE
        || simCnt
#endif
        ; )
    {
        const PimContact *cnt = 0;
         
        if(i < numContacts)
        {
            cnt = mAllContacts.at( i );
	    if( !cnt ) continue;
        }
        
#ifdef QTOPIA_PHONE
        //  Find any sim contacts that should appear before this 
        useSimContact = (simCnt && 
            (i >= numContacts || simCnt->fileAs().lower() < cnt->fileAs().lower()));
        
        if(useSimContact) 
        {
            cnt = simCnt;
            simCnt = mSimContacts.next();
        }
#endif
        
        QString fa = cnt->fileAs();

        //  Look for matching categories that should appear before this contact (UNSELECTED ONLY)
        while( catIt != mCategories.end() && fa > catName ) {
	    int fi = AbUtil::findPrefixInName( catName, filStr );
	    if( filStr.isEmpty() || fi != -1 ) {
		int matchLen = (fi != -1 ? filLen : 0 );
		matchingContacts.append( new MatchingContact( *catIt, catName, fi, matchLen ) );
		matchedCategories.append( *catIt );
            }
            
            ++catIt;
            catName = mCategoryManager.label( "Address Book", *catIt );
            if( catName[0] == '_' )
                catName = catName.mid( 1 );
        }
        
        QValueList<PimContact::ContactFields> fields;
        if(cnt->customField("BUSINESS_CONTACT").isEmpty())
            fields = mHomeFields;
        else
            fields = mBusFields;
            
	QValueList<PimContact::ContactFields>::Iterator nit;

	int fi = AbUtil::findPrefixInName( fa, filStr );
        int matchLen = (fi != -1 ? filLen : 0 );
	if( filStr.isEmpty() || fi != -1 ) { // contact's name matches
	    for( nit = fields.begin() ; nit != fields.end() ; ++nit ) {
		if( !cnt->field( (*nit) ).isEmpty() ) {
		    if( *nit == PimContact::Emails ) {
			// Special case: seperate email list field.
			QStringList emails = cnt->emailList();
			QString defaultEmail = cnt->defaultEmail();
			QStringList::Iterator def = emails.find( defaultEmail );
			if( def != emails.begin() ) {
			    emails.remove( def );
			    emails.prepend( defaultEmail );
			}                            
                            
			for( def = emails.begin(); def != emails.end(); ++def ) {
			    matchingContacts.append( new MatchingContact( cnt, *nit,
			      *def, fi, matchLen, useSimContact ) );
			    matchedContacts.append( cnt->uid() );
			}
		    } else {
			matchingContacts.append( new MatchingContact( cnt, *nit, 
			  cnt->field( *nit ), fi, matchLen, useSimContact ) );
			matchedContacts.append( cnt->uid() );
		    }
		}
	    }
	}
	else {
	    for( nit = fields.begin() ; nit != fields.end() ; ++nit ) {
                if( *nit == PimContact::Emails ) {
                    // Special case: seperate email list field.
                    QStringList emails = cnt->emailList();
                    QString defaultEmail = cnt->defaultEmail();
                    QStringList::Iterator def = emails.find( defaultEmail );
                    if( def != emails.begin() ) {
                        emails.remove( def );
                        emails.prepend( defaultEmail );
                    }                            
                    
                    for( def = emails.begin(); def != emails.end(); ++def ) {
                        if( (*def).left( filStr.length() ) == filStr ) {
                            matchingContacts.append( new MatchingContact( cnt, *nit,
                                *def, fi, matchLen, useSimContact ) );
			    matchedContacts.append( cnt->uid() );
                        }
                    }
                } else {                            
#ifdef QTOPIA_PHONE
                    if( PhoneUtils::matchPrefix( cnt->field( *nit ), filStr ) ) 
#else
                    if( cnt->field( *nit ).left( filStr.length() ) == filStr ) 
#endif
                    {
			matchingContacts.append( new MatchingContact( cnt, *nit, 
			  cnt->field( *nit ), fi, matchLen, useSimContact ) );
			matchedContacts.append( cnt->uid() );
                    }
                }
            }
	}
        
#ifdef QTOPIA_PHONE
        if(!useSimContact)
#endif
            ++i;
    }

    //  There may be other matching categories that need to appear at the end of the list...
    while( catIt != mCategories.end() ) {
        if( !mSelectedCategories.contains( *catIt ) ) {
            int fi = AbUtil::findPrefixInName( catName, filStr );
            if( filStr.isEmpty() || fi != -1 ) {
                int matchLen = (fi != -1 ? filLen : 0 );
                matchingContacts.append( new MatchingContact( *catIt, catName, fi, matchLen ) );
		matchedCategories.append( *catIt );
            }
        }

        ++catIt;
        
        catName = mCategoryManager.label( "Address Book", *catIt );
        if( catName[0] == '_' )
            catName = catName.mid( 1 );
    }

    //  Add in all selected categories
    for( catIt = mSelectedCategories.begin(); catIt != mSelectedCategories.end(); ++catIt ) {
	if ( !matchedCategories.contains( *catIt ) ) {
	    catName = mCategoryManager.label( "Address Book", *catIt );
	    if( catName[0] == '_' )
		catName = catName.mid( 1 );
	    matchingContacts.append( new MatchingContact( *catIt, catName, 0, 0 ) );
	}
    }
    
    //  Add in all selected contacts
    QValueList<SelectedContact>::ConstIterator selIt;
    for( selIt = mSelected.begin(); selIt != mSelected.end(); ++selIt )
	if ( !matchedContacts.contains( (*selIt).mContact->uid() )) {
	    matchingContacts.append( new MatchingContact( (*selIt).mContact, 
	    (*selIt).mFieldType, (*selIt).mContact->field( (*selIt).mFieldType ), 
	    0, 0, (*selIt).mSimContact ) );
	}
    
    //  List the matching numbers
    QListIterator<MatchingContact> it( matchingContacts );
    for(  ; *it ; ++it ) {
        ContactFieldListItem *newItem;
        if((*it)->fieldType == -1) {
            newItem = 
                new ContactFieldListItem( categoryPixmap(), (*it)->number,
                    (ContactFieldListItem::FieldType)-1, this, prevItem, (*it)->categoryId);
        } else {
            PimContact cnt = *(*it)->cnt;
            newItem = new ContactFieldListItem( 
                ((*it)->simContact ? simPixmap() : contactPixmap( cnt )), cnt.fileAs(),
                    (ContactFieldListItem::FieldType)(*it)->fieldType, this, 
                    prevItem, (*it)->simContact );
            int newidx = index(newItem);
            mIndexToContact[ newidx ] = (*it)->cnt;
            if( (*it)->fieldType == PimContact::Emails )
                mIndexToEmail[ newidx ] = (*it)->number;
        }
        if( (*it)->matchStart != -1 )
            newItem->setHighlight( (*it)->matchStart, (*it)->matchLength );
        
        prevItem = newItem;
    }
}

void NumberSelector::addUserEntry( const QString &number )
{
    ContactFieldListItem::FieldType fieldType = ContactFieldListItem::HomePhone;
    if( mHomeFields.count() )
        fieldType = (ContactFieldListItem::FieldType)mHomeFields.first();

    PimContact *cnt = new PimContact();
    cnt->setFirstName( "Unknown" );
    cnt->setFileAs( "Unknown" );
    cnt->setField( fieldType, number );

    mSelected += SelectedContact( cnt->fileAs(), fieldType, number, cnt, FALSE );
    mUserContacts.append( cnt );
    
    refresh();
    emit selectionChanged();
}

int NumberSelector::selectedCount()
{
    return mSelected.count() + mSelectedCategories.count();
}

QPixmap NumberSelector::categoryPixmap()
{
    QPixmap *cached = QPixmapCache::find( "addressbook/generic-category-cfl" );
    if( cached )
        return *cached;
    else {
        QPixmap gp;
        gp.convertFromImage( Image::loadPortrait(
            Resource::findPixmap( "addressbook/generic-category" ), 24, 30 ) );
        QPixmapCache::insert( "addressbook/generic-category-cfl", gp );
        return gp;
    }
}

QPixmap NumberSelector::unknownPixmap()
{
    QPixmap *cached = QPixmapCache::find( "addressbook/unknown-contact-cfl" );
    if( cached )
        return *cached;
    else {
        QPixmap gp;
        gp.convertFromImage( Image::loadPortrait(
            Resource::findPixmap( "addressbook/unknown-contact" ), 24, 30 ) );
        QPixmapCache::insert( "addressbook/unknown-contact-cfl", gp );
        return gp;
    }
}

QStringList NumberEntryDialog::numberList()
{
    return mResults;
}

QString NumberEntryDialog::number()
{
    if(mResults.count() > 0)
        return mResults[0];
    else
        return QString("");
}

QString NumberSelector::extraInfoText(int idx) const
{
    if( mIndexToEmail.contains( idx ) )
        return mIndexToEmail[idx];
    else
        return fieldDataAt( idx );
}

void NumberEntryDialog::addToCategory()
{
    mNumberList->addSelectedToCategory();
}
#include <qmessagebox.h>
void NumberSelector::addSelectedToCategory()
{
    if( selectedCount() )
    {
        CategorySelectDialog *categoryDlg = new CategorySelectDialog( "Address Book" , 
            this, 0, TRUE);
        categoryDlg->setUnfiled(FALSE);
        categoryDlg->addCreateOption();
        if( QPEApplication::execDialog(categoryDlg) == QDialog::Accepted )
        {    
            QList<PimContact> selected = selectedContacts();
            int numSelected = selected.count();
            int newCat = categoryDlg->currentCategory();
            mCategoryManager.load( categoryFileName() );
            
            for( int i = 0; i < numSelected; ++i )
            {
                PimContact *cnt = selected.at( i );
                QArray<int> cats = cnt->categories();
                int size = cats.size();

                if( !cats.contains( newCat ) )
                {                
                    cats.resize( size + 1 );
                    cats[size] = newCat;
                    cnt->setCategories( cats );
                    mAddressBook.updateContact( *cnt );
                }
            }
            contacts.saveData();
            
            mSelected.clear();
            mSelectedCategories.clear();
            
            if( !mCategories.contains( newCat ) )
                mCategories += newCat;
                
            mSelectedCategories += newCat;
            
            refresh();
        }   
        delete categoryDlg;
    }
}



