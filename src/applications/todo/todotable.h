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

#include <qtable.h>
#include <qmap.h>
#include <qguardedptr.h>
#include <qtopia/categories.h>
#include <qtopia/pim/task.h>
#include <qtopia/pim/private/todoxmlio_p.h>

class QTimer;

class TablePrivate;

#if defined(QTOPIA_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QTOPIAPIM_EXPORT SortedRecords<PimTask>;
// MOC_SKIP_END
#endif

class TodoTable : public QTable
{
    Q_OBJECT

public:
    TodoTable( const SortedTasks &tasks, QWidget *parent = 0, const char * name = 0, const char *appPath=0 );
    ~TodoTable();

    void clearFindRow() { currFindRow = -2; }

    bool hasCurrentEntry();
    PimTask currentEntry();
    void setCurrentEntry(QUuid &);

    QString categoryLabel( int id );

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
    QValueList<QUuid> selectedTasks();
    QValueList<PimTask> selected();
    void selectAll();
 
    void setFields(QValueList<int> f);
    QValueList<int> fields();
    
    static QString statusToText(PimTask::TaskStatus s);

public slots:
    void slotDoFind( const QString &findString, int category );
    void reload(const SortedTasks &tasks);

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
    void setFields(QValueList<int> f, QStringList sizes);
    int defaultFieldSize(PimTask::TaskFields f);
    QValueList<int> defaultFields();

    void keyPressEvent( QKeyEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );

    int rowHeight( int ) const;
    int rowPos( int row ) const;
    int rowAt( int pos ) const;

    void fontChange( const QFont & );

    QWidget *createEditor(int,int,bool) const;
    void setCellContentFromEditor(int,int);

private slots:
    void cornerButtonClicked();
    void refresh();
    void setCellContentFromEditor();
    void slotClicked( int row, int col, int button, const QPoint &pos );
    void slotPressed( int row, int col, int button, const QPoint &pos );
    void slotDoubleClicked(int, int, int, const QPoint &);
    void slotCurrentChanged(int row, int col );
    void rowHeightChanged( int row );
    void priorityChanged(int);

    void headerClicked(int);

private:
    void readSettings();
    void saveSettings();

    void setSelection(int fromRow, int toRow);
    void setSelection(int row);
    int pos(const QUuid &);


private:
    Categories mCat;
    int currFindRow;
    
    SortedTasks mTasks;
    QStringList categoryList;
    QTimer *menuTimer;
    QString currFindString;
    SelectionMode mSel;
    QValueList<QUuid> mSelected;
    
    QValueList<int> headerKeyFields;

    TablePrivate *d;
    int mSortColumn;
};

#endif
