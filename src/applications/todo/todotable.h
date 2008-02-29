/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TODOTABLE_H
#define TODOTABLE_H

#include <QTableView>
#include <QList>
#include "taskdelegate.h"

class TodoTable : public QTableView
{
    Q_OBJECT

public:
    TodoTable( QWidget *parent = 0 );
    ~TodoTable();

    void setModel( QAbstractItemModel * model );

    QTask currentTask() const
    {
        if (taskModel() && currentIndex().isValid())
            return taskModel()->task(currentIndex());
        return QTask();
    }

    QList<QTask> selectedTasks() const;
    QList<QUniqueId> selectedTaskIds() const;

    QTaskModel *taskModel() const { return qobject_cast<QTaskModel *>(model()); }

    TaskTableDelegate *taskDelegate() const { return qobject_cast<TaskTableDelegate *>(itemDelegate()); }

signals:
    void currentItemChanged(const QModelIndex &);
    void taskActivated(const QTask &);

protected:
    void resizeEvent(QResizeEvent * e);
    virtual void keyPressEvent(QKeyEvent *);

protected slots:
    void currentChanged(const QModelIndex &, const QModelIndex &);
    void showTask(const QModelIndex &);
    void reset();
    void toggleTaskCompleted(const QModelIndex &i);

private:
    void updateColumnSizes();
    // since needs to treat as list, not spreadsheet.
    void setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior)
    { QTableView::setSelectionBehavior(behavior); }

    QUniqueId lastSelectedTaskId;
};

#endif
