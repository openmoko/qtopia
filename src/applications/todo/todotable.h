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

#ifndef TODOTABLE_H
#define TODOTABLE_H

#include <qpe/categories.h>
#include <qpe/stringutil.h>
#include <qpe/pim/task.h>
#include <qpe/pim/private/todoxmlio_p.h>

#include <qtable.h>
#include <qmap.h>
#include <qguardedptr.h>

class QTimer;

class TodoTable : public QTable
{
    Q_OBJECT

public:
    TodoTable( QWidget *parent = 0, const char * name = 0 );
    void addEntry( const PimTask &todo );
    void removeEntry(const PimTask &todo );
    void updateEntry(const PimTask &todo );
    void clearFindRow() { currFindRow = -2; }

    PimTask currentEntry();
    void setCurrentEntry(QUuid &);

    QString categoryLabel( int id );

    void setShowCompleted( bool sc );
    bool showCompleted() const;

    void setShowCategory( int c );
    int showCategory() const;

    void removeCurrentEntry();

    virtual void sortColumn( int col, bool ascending, bool /*wholeRows*/ );

    void paintCell(QPainter *p, int row, int col, 
	    const QRect &, bool);

    void resizeData(int) { }
    QTableItem * item(int, int) const { return 0; }
    void setItem(int,int,QTableItem*) { }
    void clearCell(int,int) { }


public slots:
    void slotDoFind( const QString &findString, int category );
    void refresh();
    void reload();
    void flush();

signals:
    void signalEdit();
    void signalDoneChanged( bool b );
    void signalPriorityChanged( int i );
    void signalShowMenu( const QPoint & );
    void findNotFound();
    void findWrapAround();
    void findFound();

protected:
    void keyPressEvent( QKeyEvent *e );

    int rowHeight( int ) const;
    int rowPos( int row ) const;
    int rowAt( int pos ) const;

    QWidget *createEditor(int,int,bool) const;
    void setCellContentFromEditor(int,int);

private slots:
    void setCellContentFromEditor();
    void slotClicked( int row, int col, int button, const QPoint &pos );
    void slotPressed( int row, int col, int button, const QPoint &pos );
    void slotCurrentChanged(int row, int col );
    void slotShowMenu();
    void rowHeightChanged( int row );
    void priorityChanged(int);

private:
    friend class TodoWindow;
    mutable TodoXmlIO ta;

    QStringList categoryList;
    QTimer *menuTimer;
    Categories mCat;
    int currFindRow;
    QString currFindString;
};

#endif
