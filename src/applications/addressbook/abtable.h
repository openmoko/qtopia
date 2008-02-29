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

#ifndef ABTABLE_H
#define ABTABLE_H

#include <qtopia/categories.h>
#include <qtopia/pim/contact.h>
#include <qtopia/pim/private/contactxmlio_p.h>

#include <qmap.h>
#include <qtable.h>
#include <qstringlist.h>
#include <qcombobox.h>

class AbTable : public QTable
{
    Q_OBJECT

public:
    static const int FREQ_CONTACT_FIELD;

    AbTable(ContactXmlIO *,  QWidget *parent, const char *name=0, const char *appPath=0 );
    ~AbTable();

    bool hasCurrentEntry();
    PimContact currentEntry();
    void setCurrentEntry(const QUuid &u);

    // sets the prefered field
    void setPreferredField( int key );

    void clearFindRow() { currFindRow = -2; }

    enum SelectionMode {
	NoSelection,
	Single,
	Extended
    };
    
    void setSelectionMode(SelectionMode m);
    SelectionMode selectionMode() { return mSel; }
    
    QValueList<PimContact> selected();
    QValueList<QUuid> selectedContacts();
    void selectAll();

    void setFields(QValueList<int> f);
    QValueList<int> fields();

    void setCurrentCell(int, int);

public slots:
    void reload();
    void findNext( const QString &str, int category );
    void fitHeadersToWidth();

signals:
    void currentChanged();
    void clicked();
    void doubleClicked();
    void findNotFound();
    void findWrapAround();
    void findFound();

protected:
    void setFields(QValueList<int> f, QStringList);
    int defaultFieldSize(PimContact::ContactFields f);
    QValueList<int> defaultFields();
    int minimumFieldSize(PimContact::ContactFields);
    void calcFieldSizes(int oldSize, int size);
    
    virtual void keyPressEvent( QKeyEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void resizeEvent( QResizeEvent *e );
    void showEvent( QShowEvent *e);

    void fontChange(const QFont &);
     int rowHeight( int ) const;
     int rowPos( int row ) const;
     virtual int rowAt( int pos ) const;

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
    PimContact pimForUid(const QUuid &);
    void saveSettings();
    
    QString findContactName( const PimContact &entry );
    QString findContactContact( const PimContact &entry );
    QString getField( const PimContact &entry, int );

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


    ContactXmlIO *contacts;
    int currFindRow;
    QString currFindString;
    QStringList choicenames;
    bool enablePainting;
    int showCat;
    int prefField;

    QValueList<int> headerKeyFields;
    
    SelectionMode mSel;
    QValueList<QUuid> mSelected;
    int mSortColumn;
    bool mAscending;
};
#endif // ABTABLE_H
