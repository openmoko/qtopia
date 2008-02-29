/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#ifndef __TASKVIEW_H__
#define __TASKVIEW_H__

#include <qtopia/pim/qtask.h>
#include <qtopia/pim/qtaskmodel.h>

#include <QListView>
#include <QAbstractItemDelegate>
#include <QMap>
#include <QDialog>

class QFont;
class QTOPIAPIM_EXPORT QTaskDelegate : public QAbstractItemDelegate
{
public:
    explicit QTaskDelegate( QObject * parent = 0 );
    virtual ~QTaskDelegate();

    virtual void paint( QPainter *painter,
                        const QStyleOptionViewItem & option,
                        const QModelIndex & index ) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option,
                           const QModelIndex &index) const;

    virtual QFont mainFont(const QStyleOptionViewItem &) const;

private:
    QFont differentFont(const QFont& start, int step) const;
};

class QTOPIAPIM_EXPORT QTaskListView : public QListView
{
    Q_OBJECT

public:
    explicit QTaskListView(QWidget *parent);
    ~QTaskListView();

    void setModel( QAbstractItemModel * );

    QTask currentTask() const
    {
        if (taskModel() && currentIndex().isValid())
            return taskModel()->task(currentIndex());
        return QTask();
    }

    QList<QTask> selectedTasks() const;
    QList<QUniqueId> selectedTaskIds() const;

    QTaskModel *taskModel() const { return qobject_cast<QTaskModel *>(model()); }

    QTaskDelegate *taskDelegate() const { return qobject_cast<QTaskDelegate *>(itemDelegate()); }
};

class QTaskSelectorPrivate;
class QTOPIAPIM_EXPORT QTaskSelector : public QDialog
{
    Q_OBJECT
public:
    QTaskSelector(bool allowNew, QWidget *);
    void setModel(QTaskModel *);

    bool newTaskSelected() const;
    bool taskSelected() const;
    QTask selectedTask() const;

private slots:
    void setNewSelected();
    void setSelected(const QModelIndex&);

private:
    QTaskSelectorPrivate *d;
};

#endif//__TASKVIEW_H__
