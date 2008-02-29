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

#ifndef HAVE_CONTACTFIELDLIST_H
#define HAVE_CONTACTFIELDLIST_H

#include <qlistbox.h>

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
    None = PimContact::ContactFieldsEnd+1
};
#define FIELDTYPE FieldType
#else
#define FIELDTYPE ContactFieldListItem::FieldType
#endif

class ContactFieldListItem : public QListBoxItem
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
	None = PimContact::ContactFieldsEnd+1
    };


    ContactFieldListItem( const QPixmap &contactImage, 
	    const QString &t, FIELDTYPE fieldType, 
	    QListBox *lb, QListBoxItem *after = 0 );
    ContactFieldListItem( QListBox *lb );
    ContactFieldListItem( QListBox *lb, QListBoxItem *after );

    QString id() const;

    void setHighlight( int index, int len );

    void setContactPix( const QPixmap &contactPix );
    void setFieldType( ContactFieldListItem::FieldType fieldType );
    ContactFieldListItem::FieldType fieldType() const;

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
};

class ContactFieldList : public QListBox
{
    friend class ContactFieldListItem;
    Q_OBJECT
public:
    ContactFieldList( QWidget *parent, const char *name = 0, WFlags fl = 0 );

    void setSelectedFields( const QStringList &fieldData );
    QStringList unknownFieldData() const;
    virtual void setAllowMultiple( bool allowMultiple );
    bool allowMultiple() const;
    void setSelectionMode( QListBox::SelectionMode m );
    QValueList<int> selected() const;
    PimContact contactAt( int idx ) const;
    int fieldTypeAt( int idx ) const;
    QString fieldDataAt( int idx ) const;

    QString filter() const;
public slots:
    virtual void refresh();
    void setFilter( const QString &f );
    virtual void reset();

protected slots:
    void contactsChanged();
    void itemClicked( int idx );
    void itemClicked( QListBoxItem * );

protected:
    virtual QString extraInfoText( int idx ) const;
    void viewportPaintEvent( QPaintEvent *e );
    void paletteChange( const QPalette &pal );
    void focusInEvent( QFocusEvent *e );

    QPixmap contactPixmap( const PimContact &cnt );
    QPixmap fieldTypePixmap( ContactFieldListItem::FieldType Type );

    QString mFilter;

    QValueList<QString> mSelected;

    QMap<int, const PimContact *> mIndexToContact;
    ContactXmlIO contacts;
    SortedContacts mAllContacts;

    QColor mStripeColour, mDimTextColour;

    bool mAllowMultiple;

    QStringList mUnknownFieldData;
    QString m_noMatches;
};

#endif
