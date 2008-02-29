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
#ifndef ABTABLE_H
#define ABTABLE_H

//AbTable should _not_ be public with contactxmlio_p.h included here
#include <qtopia/pim/private/contactxmlio_p.h>

#include <qtopia/categories.h>
#include <qtopia/pim/contact.h>
#include <qtopia/qpeglobal.h>
#include <qdialog.h>

#include <qlayout.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qtable.h>
#include <qstringlist.h>
#include <qcombobox.h>

#include <qtopia/pim/qtopiapimwinexport.h>

#ifdef QTOPIA_PHONE
#include <qtopia/phone/phone.h>
#include <qtopia/phone/phonebook.h>
#include <qtopia/pim/private/numberentry_p.h>
#endif

#ifndef QTOPIA_DESKTOP
#define PACK_IN_1_COLUMN
#endif

class ContactXmlIO;

class QTOPIAPIM_EXPORT AbTable : public QTable
{
    Q_OBJECT

public:
    static const int FREQ_CONTACT_FIELD;

    AbTable(ContactXmlIO *,  QWidget *parent, const char *name=0, const char *appPath=0 );
    ~AbTable();

    bool hasCurrentEntry() const;
    PimContact currentEntry() const;
#ifdef QTOPIA_PHONE
    void setCurrentSimEntry( const QString &fa );
    QString currentSimEntry() const;
#endif
    void setCurrentEntry(const QUuid &u);

    /* simply disabled for now although this might be useful
    // sets the prefered field
    void setPreferredField( int key );
    */


    enum SelectionMode {
	NoSelection,
	Single,
	Extended
    };
    
    void setSelectionMode(SelectionMode m);
    SelectionMode selectionMode() const { return mSel; }
    
    QValueList<PimContact> selected() const;
    QValueList<PimContact> all() const;
    QValueList<QUuid> selectedContacts() const;
    void selectAll();

    void setFields(QValueList<int> f);
    QValueList<int> fields();

    void setCurrentCell(int, int);

    void setKeyChars(const QString&);

#ifdef QTOPIA_PHONE
    QMap<QString,PimContact *> existingContactSimData() const;
    void setSimContacts( const QList<PimContact> &simContacts );
#endif

public slots:

    void reload();
    void fitToSize();

signals:
    void currentChanged();
    void clicked();
    void doubleClicked();

protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void setFields(QValueList<int> f, QStringList);
    QValueList<int> defaultFields();
#ifndef PACK_IN_1_COLUMN
    int defaultFieldSize(PimContact::ContactFields f);
    int minimumFieldSize(PimContact::ContactFields);
#endif
    void calcFieldSizes(int oldSize, int size);
    
    virtual void keyPressEvent( QKeyEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void resizeEvent( QResizeEvent *e );
    void showEvent( QShowEvent *e);
    
    void setVBarGeometry(QScrollBar& vbar, int x, int y, int w, int h );

    void fontChange(const QFont &);
    int rowHeight( int ) const;
    int rowPos( int row ) const;
    virtual int rowAt( int pos ) const;

#ifdef PACK_IN_1_COLUMN
    void paletteChange( const QPalette & );
#endif

protected slots:
    void moveTo( const QString & );
    virtual void columnClicked( int col );
    void itemClicked(int,int col);
    void slotDoubleClicked(int, int, int, const QPoint &);
    void rowHeightChanged( int row );

    void slotCurrentChanged(int row, int col );
    void readSettings();
private:
    void refresh();
    
    void setSelection(int fromRow, int toRow);
    PimContact pimForUid(const QUuid &) const;
    void saveSettings();
    
    QString findContactName( const PimContact &entry );
    QString findContactContact( const PimContact &entry );
    QString getField( const PimContact &entry, int );
    QString fieldText(const PimContact& c, int key);

    // paint related functions
    void show();
    void setPaintingEnabled( bool e );

    void paintFocus( QPainter *, const QRect & ) {}

    void paintCell ( QPainter * p, int row, int col,
	    const QRect & cr, bool selected );

    void resizeData(int) { }
    QTableItem * item(int, int) const { return 0; }
    void setItem(int,int,QTableItem*) { }
    void clearCell(int,int) { }
    QWidget *createEditor(int,int,bool) const { return 0;}

#ifdef QTOPIA_PHONE
    QList<PimContact> mSimContacts;
    QMap<QString,PimContact *> mExistingContactSimData;
    QString mCurSimEntry;
#endif

    ContactXmlIO *contacts;
    SortedContacts mSortedContacts;
    SortedContacts mAllContacts;
    QStringList choicenames;
    bool enablePainting;
    int showCat;
    int prefField;
    int rowH;
#ifdef PACK_IN_1_COLUMN
    QFont fnt[3];
    static QFont differentFont(const QFont& start, int step);
    QBrush stripebrush;
    QPen dimpen;
    QPixmap generic;
    QPixmap sim;
#endif

    QValueList<int> headerKeyFields;
    
    SelectionMode mSel;
    QValueList<QUuid> mSelected;
    int mSortColumn;
    bool mAscending;

private:
    QString keychars;
};

//------------------------------------------------------------------------------

class QTOPIAPIM_EXPORT ContactSelector : public QDialog
{
    Q_OBJECT
public:
    ContactSelector( bool allowNew, ContactXmlIO *contacts, QWidget *parent = 0, 
			const char *name = 0, bool modal = TRUE, WFlags fl = 0 );

    QValueList<PimContact> selected() const;

    QSize sizeHint() const;

    AbTable *table;

    enum SelectTypes 
    {
	NoSelection,
	SelectedContacts,
	SelectedNew
    };

    SelectTypes selectedType() const;

protected:
    void setSelectedType( SelectTypes t );
protected slots:
    void accept();
    void newSelected();
    void contactSelected();
private:
    SelectTypes mSelectedType;
};

//------------------------------------------------------------------------------

class QLabel;
class QListView;
class QTOPIAPIM_EXPORT PhoneTypeSelector : public QDialog
{
    Q_OBJECT
public:
    PhoneTypeSelector( const PimContact &cnt, const bool showEmpty, 
        const QString &labelText, QWidget *parent, const char *name = 0, 
        bool modal = TRUE, WFlags fl = 0 );
    int selected() const;
protected slots:
    void accept();
signals:
    void selected(PimContact::ContactFields);
private:
    //QString verboseIfEmpty( const QString &number );
    QListViewItem* PhoneTypeSelector::addType(QListViewItem* prevItem,
        QString number, const char* pixmapName, PimContact::ContactFields phoneType);

    bool mShowEmpty;
    QLabel *mLabel;
    QListView *mPhoneType;
    QMap<QListViewItem *, PimContact::ContactFields> mItemToPhoneType;
    const PimContact mContact;
};

#ifdef QTOPIA_PHONE
class AddressBookAccess;
#endif
class QTOPIAPIM_EXPORT AbUtil
{
    //utility functions for the addressbook
public:
    static const int ImageHeight;
    static const int ImageWidth;

    static int findPrefixInName( const QString &name, const QString &prefix );

    //compare the data of two contacts. returns TRUE if they are equivalent, FALSE otherwise.
    static bool compareContacts( const PimContact &cnt1, const PimContact &cnt2 );

#ifdef QTOPIA_PHONE
    //these make iterating through the phone fields easy
    static const int phoneFields[];
    static const int numPhoneFields;

    //sim keys- used to access the pim contact sim fields (custom fields)
    static const QString SIM_HP, SIM_HM, SIM_HF, SIM_BP, SIM_BM, SIM_BF, SIM_BPA;
    static const QChar SIM_PARTIAL_INDICATOR;
    static QString simKeyForPhoneType( int phoneType );

    static int simTypeTextToPhoneType( const QString &typeText );
    static QString simTypeTextForPhoneType( const int &phoneType );
    static QString generateSimText( const QString &name, const int &phoneType );
    static void parseSimText( QString &name, int &phoneType );

    static uint numbersForSimCount( const PimContact &cnt );
    /* returns true if cnt has numbers that need to be stored on the sim */
    static bool hasNumbersForSim( const PimContact &cnt );

    /* set the limits of the fields on the sim */
    static void setSimFieldLimits( const QString &limits );
    static int SIM_TEXT_LIMIT, SIM_NUMBER_LIMIT;

    //converts a list of phonebookentries into contacts, performing integration
    static QList<PimContact> phoneBookToContacts( const 			   
        QValueList<PhoneBookEntry>& list, bool sort = FALSE);
    //syncs a contact with the phonebook entries on the sim
    static void syncContactWithSim( PimContact &cnt, 
		QValueList<PhoneBookEntry> &curSimList, PhoneBook *pb = 0 );

    /* finds a contact given a phone number. puts the kind of phone number
       that matches into the field pointer argument */
    static PimContact findContactByNumber( const QString& number, int *field, 
					    AddressBookAccess *access = 0 );

    static int phoneKeyMatchN(const QString& haystack, int offset, const QString& needle, int n );

    //merging of sim contacts into database contacts
    static void unmergeSimContact( PimContact &cnt );
    static void mergeSimContact( PimContact &cnt, const PimContact &simContact );
#endif
};

#endif // ABTABLE_H
