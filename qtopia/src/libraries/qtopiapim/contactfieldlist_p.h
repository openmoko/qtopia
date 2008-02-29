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

#ifndef HAVE_CONTACTFIELDLIST_H
#define HAVE_CONTACTFIELDLIST_H

#include <qlistbox.h>

#ifdef QTOPIA_PHONE
#include <qtopia/phone/phone.h>
#include <qtopia/phone/phonebook.h>
#else
#ifndef PHONELINE_HACK
#define PHONELINE_HACK
class PhoneLine { public: enum QueryType {}; };
class PhoneBookEntry {};
#endif
#endif

#include <qtopia/pim/private/contactxmlio_p.h>

#if defined(Q_OS_WIN32) && _MSC_VER < 1300
enum FieldType
{
    HomePhone = PimContact::HomePhone,
    HomeMobile = PimContact::HomeMobile,
    HomeFax = PimContact::HomeFax,
    BusinessPhone = PimContact::BusinessPhone,
    BusinessMobile = PimContact::BusinessMobile,
    BusinessFax = PimContact::BusinessFax,
    BusinessPager = PimContact::BusinessPager,
    Email = PimContact::Emails,
    None = PimContact::ContactFieldsEnd+1,
    Category = -1
};
#define FIELDTYPE FieldType
#else
#define FIELDTYPE ContactFieldListItem::FieldType
#endif

class QTOPIAPIM_EXPORT MatchingContact
{
public:
    MatchingContact( const PimContact *c, int ft, const QString &n, int start, int len, 
        bool simCnt = FALSE)
    : cnt( c ), fieldType( ft ), number( n ), matchStart( start ), matchLength( len ),
        simContact( simCnt ), categoryId( -1 )
    {
    }
    
    MatchingContact( const int catId, const QString &n, int start, int len )
    : cnt( 0 ), fieldType( -1 ), number( n ), matchStart( start ), matchLength( len ),
        simContact( FALSE ), categoryId( catId )
    {
    }

    const PimContact *cnt;
    int fieldType;
    QString number;
    int matchStart, matchLength;
    bool simContact;
    int categoryId;
};

class QTOPIAPIM_EXPORT SelectedContact
{
public:
    SelectedContact()
    : mContact( 0 ), mFieldType( -1 ) 
    {}
    
    SelectedContact( const QString &text, int fieldType, 
                const QString &extraInfoText, const PimContact *c = 0, bool simContact = FALSE )
    : mContact( c ), mText( text ), mFieldType( fieldType ), mExtraInfoText( extraInfoText ), 
                mSimContact( simContact )
    {}
    
    SelectedContact( const SelectedContact &other )
    {
        mContact = other.mContact;
        mFieldType = other.mFieldType;
        mText = other.mText;
        mExtraInfoText = other.mExtraInfoText;
        mSimContact = other.mSimContact;
    }
    
    bool operator==( const SelectedContact &other ) const
    {
        return (mContact == other.mContact && mFieldType == other.mFieldType && 
            mText == other.mText && mExtraInfoText == other.mExtraInfoText && 
            mSimContact == other.mSimContact );
    }

    const PimContact *mContact;
    QString mText;
    int mFieldType;
    QString mExtraInfoText;
    bool mSimContact;
};

class QTOPIAPIM_EXPORT ContactFieldListItem : public QListBoxItem
{
public:
    enum FieldType
    {
	HomePhone = PimContact::HomePhone,
	HomeMobile = PimContact::HomeMobile,
	HomeFax = PimContact::HomeFax,
	BusinessPhone = PimContact::BusinessPhone,
	BusinessMobile = PimContact::BusinessMobile,
	BusinessFax = PimContact::BusinessFax,
	BusinessPager = PimContact::BusinessPager,
	Email = PimContact::Emails,
	None = PimContact::ContactFieldsEnd+1,
        Category = -1
    };


    ContactFieldListItem( const QPixmap &contactImage, 
	    const QString &t, FIELDTYPE fieldType, 
	    QListBox *lb, QListBoxItem *after = 0, int catId = -1,
            bool simContact = FALSE );
    ContactFieldListItem( QListBox *lb );
    ContactFieldListItem( QListBox *lb, QListBoxItem *after );

    SelectedContact id() const;

    void setHighlight( int index, int len );
    
    void setContactPix( const QPixmap &contactPix );
    void setFieldType( ContactFieldListItem::FieldType fieldType );
    ContactFieldListItem::FieldType fieldType() const;
    int categoryId() const;
    
    virtual int width( const QListBox *lb ) const;
    virtual int height( const QListBox *lb ) const;
protected:
    virtual void paint( QPainter *p );
private:
    void init();

    ContactFieldListItem::FieldType mFieldType;

    QPixmap mContactPix, mFieldTypePix;

    int mHighlightIndex, mHighlightLength;
    bool mHighlight;
    int mCategoryId;
    bool mSimContact;
};

class QTOPIAPIM_EXPORT ContactFieldList : public QListBox
{
    friend class ContactFieldListItem;
    Q_OBJECT
public:
    ContactFieldList( QWidget *parent, const char *name = 0, WFlags fl = 0 );
    virtual ~ContactFieldList();
    
    void setSelectedFields( const QStringList &fieldData );
    QStringList unknownFieldData() const;
    virtual void setAllowMultiple( bool allowMultiple );
    bool allowMultiple() const;
    void setSelectionMode( QListBox::SelectionMode m );
    QValueList<int> selected() const;
    PimContact contactAt( int idx ) const;
    int fieldTypeAt( int idx ) const;
    QString fieldDataAt( int idx ) const;

    void loadSimContacts();
        
    QString filter() const;
    
    QString emptyMessage();
    void setEmptyMessage(QString msg);
    
    bool isSelected( const PimContact *contact, int fieldType ) const;
   
public slots:
    virtual void refresh();
    void setFilter( const QString &f );
    virtual void reset();
    void phonebookChanged(const PhoneLine&, const QString&, const QValueList<PhoneBookEntry> &list);
    
protected slots:
    void contactsChanged();
    void itemClicked( int idx );
    void itemClicked( QListBoxItem * );

signals:
    void gotSimContacts();
    void selectionChanged();
    
protected:
    virtual QString extraInfoText( int idx ) const;
    void viewportPaintEvent( QPaintEvent *e );
    void paletteChange( const QPalette &pal );
    void focusInEvent( QFocusEvent *e );

    QPixmap contactPixmap( const PimContact &cnt );
    QPixmap simPixmap();
    QPixmap fieldTypePixmap( ContactFieldListItem::FieldType Type );

    QString mFilter;

    QValueList<SelectedContact> mSelected;
    QValueList<int> mSelectedCategories;

    QMap<int, const PimContact *> mIndexToContact;
    ContactXmlIO contacts;
    SortedContacts mAllContacts;
    
#ifdef QTOPIA_PHONE
    PhoneLine *mPhoneLine;
    QList<PimContact> mSimContacts;
    bool mSimContactsLoaded;
    bool mSimContactsRequested;
#endif

    QBrush mStripeBrush;
    QColor mDimTextColour;

    bool mAllowMultiple;

    QStringList mUnknownFieldData;
    QString m_noMatches;
    
    QString mEmptyMessage;
};

#endif
