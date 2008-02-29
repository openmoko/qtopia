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

#include <qpe/categories.h>
#include <qpe/pim/contact.h>
//#include <qpe/pim/addressbookaccess.h>
#include <qpe/pim/private/contactxmlio_p.h>

#include <qmap.h>
#include <qtable.h>
#include <qstringlist.h>
#include <qcombobox.h>

class AbTable : public QTable
{
    Q_OBJECT

public:
    AbTable( const QValueList<int> *ordered, QWidget *parent, const char *name=0 );
    ~AbTable();

    void addEntry( const PimContact &newContact );
    PimContact currentEntry();
    void updateEntry( const PimContact &newContact );
    void deleteEntry( const PimContact &newContact );
    void deleteCurrentEntry();

    void setCurrentEntry(QUuid &u);

    bool hasEntry(const PimContact &c);
    
    void init();

    void clear();
    void clearFindRow() { currFindRow = -2; }
    void loadFields();

    // addresspicker mode
    void setChoiceNames( const QStringList& list);
    QStringList choiceNames() const;
    void setChoiceSelection(int index, const QStringList& list);
    QStringList choiceSelection(int index) const;
    void setShowCategory( int c );
    int showCategory() const { return showCat; }
    QString AbTable::categoryLabel( int id );
    QStringList categories();
    void setPreferredField( int id );

    void show();
    void setPaintingEnabled( bool e );

    void paintFocus( QPainter *, const QRect & ) {}

    void paintCell ( QPainter * p, int row, int col, 
	    const QRect & cr, bool selected );

    void setCurrentCell(int, int);

    void resizeData(int) { }
    QTableItem * item(int, int) const { return 0; }
    void setItem(int,int,QTableItem*) { }
    void clearCell(int,int) { }
    QWidget *createEditor(int,int,bool) const { return 0;}
public slots:
    void slotDoFind( const QString &str, int category );
    void refresh();
    void reload();
    void flush();

signals:
    void empty( bool );
    void details();
    void findNotFound();
    void findWrapAround();
    void findFound();

protected:
    virtual void keyPressEvent( QKeyEvent *e );

     int rowHeight( int ) const;
     int rowPos( int row ) const;
     virtual int rowAt( int pos ) const;

protected slots:
    void moveTo( const QString & );
    virtual void columnClicked( int col );
    void itemClicked(int,int col);
    void rowHeightChanged( int row );

private:
    void fitColumns();
    void resort();
    QString findContactName( const PimContact &entry );
    QString findContactContact( const PimContact &entry );
    QString getField( const PimContact &entry, int );

    int lastSortCol;
    bool asc;
    ContactXmlIO aba;
    const QValueList<int> *intFields;
    int currFindRow;
    QString currFindString;
    QStringList choicenames;
    bool enablePainting;
    Categories mCat;
    int showCat;
    int prefField;
};
#endif // ABTABLE_H
