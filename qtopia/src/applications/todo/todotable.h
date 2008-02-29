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

#ifndef TODOTABLE_H
#define TODOTABLE_H

#include <qtable.h>
#include <qmap.h>
#include <qguardedptr.h>
#include <qtopia/categories.h>
#include <qtopia/pim/task.h>
#include <qtopia/pim/private/todoxmlio_p.h>

class QTimer;
class PriorityEdit;
class PrTask;

class TablePrivate;

class TodoTable : public QTable
{
    Q_OBJECT

public:
    TodoTable( QWidget *parent = 0, const char * name = 0, const char *appPath=0, bool readonly = FALSE );
    ~TodoTable();

    bool hasCurrentEntry();
    PimTask currentEntry();
#ifdef QTOPIA_PHONE
    void setCurrentCell(int, int);
#endif
    void setCurrentEntry(const QUuid &u );
    void clearFindRow() {
	searchResults.clear();
	currFind = searchResults.end();
    }

    bool categoriesChanged(const QArray<int> &);
    QString categoryLabel( int id );

    void setCompletedFilter(bool);
    bool completedFilter() const;

    void setFilter(int);
    int filter() const;
    virtual void sortColumn( int col, bool , bool);

    void paintFocus(QPainter *p, const QRect &r);
    void paintCell(QPainter *p, int row, int col, 
	    const QRect &, bool);

    void resizeData(int) { }
    QTableItem * item(int, int) const { return 0; }
    void setItem(int,int,QTableItem*) { }
    void clearCell(int,int) { }
    void clearCellWidget(int, int);

    enum SelectionMode {
	NoSelection,
	Single,
	Extended
    };
    
    void setSelectionMode(SelectionMode m);
    SelectionMode selectionMode() { return mSel; }
    QValueList<int> selectedTasks();
    QValueList<PimTask> selected();
    void selectAll();
 
    void setFields(QValueList<int> f);
    QValueList<int> fields();
    
    static QString statusToText(PimTask::TaskStatus s);

    void removeList(const QValueList<int> &t);
    QSize sizeHint() const { return QSize(QTable::sizeHint().width(), rowHeight(0)*10);}

    const QList<PrTask>& tasks() { return mTasks->tasks(); }

public slots:
    void addEntry(const PimTask &todo, bool = TRUE);
    void removeEntry(const PimTask &todo);
    void updateEntry(const PimTask &todo);

    // finds from current row
    void find( const QString &findString );
    // finds from current result, previous search string.
    void findNext();
    void reload();
    void loadData();
    void saveData(bool force = FALSE);
    void fitHeadersToWidth();

signals:
    void updateTask(const PimTask &);
    void currentChanged();

    void clicked();
    void doubleClicked();
    void pressed();

    void findNotFound();
    void findWrapAround();
    void findFound();

protected:
    QWidget* priorityEdit(int, int);
    void setFields(QValueList<int> f, QStringList sizes);
    void calcFieldSizes(int oldSize, int);
    int defaultFieldSize(PimTask::TaskFields f);
    int minimumFieldSize(PimTask::TaskFields f);
    QValueList<int> defaultFields();
    
    void keyPressEvent( QKeyEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void resizeEvent( QResizeEvent *e );
    void showEvent( QShowEvent *e);

    int rowPos( int row ) const;
    int rowAt( int pos ) const;
    int rowHeight( int ) const;

    void fontChange( const QFont & );

    QWidget *createEditor(int,int,bool) const;
    void setCellContentFromEditor(int,int);

private slots:
    void cornerButtonClicked();
    void refresh();
    void delaySetCellContentFromEditor();
    void setCellContentFromEditor();
    void delayCancelEdit();
    void cancelEdit();
    void activateCell( int row, int col, int button, const QPoint &pos );
    void startMenuTimer( int row, int col, int button, const QPoint &pos );
    void refreshCell(int row, int col );
    void rowHeightChanged( int row );

    void priorityChanged(int);

    void headerClicked(int);
    void readSettings();

private:
    void toggleTaskCompletion(int row);
    void saveSettings();

    void setSelection(int fromRow, int toRow);
    int pos(const int);

    void refreshRows(int start, int end);

    int filteredAt(int) const;

private:
    Categories mCat;
    
    TodoXmlIO *mTasks;
    QStringList categoryList;
    QTimer *menuTimer;
    SelectionMode mSel;
    mutable QArray<int> mFiltered;
    mutable QArray<bool> mFilteredLoaded;

    int mLastRowShown;
    PimTask mLastTaskShown;
    
    QValueList<int> headerKeyFields;
    QValueList<int>::Iterator currFind;
    QValueList<int> searchResults;


    TablePrivate *d;
    int mSortColumn;
    bool ascSort;
    PriorityEdit *le;
    bool ro;
};

#endif
