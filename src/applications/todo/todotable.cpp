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

#include "todotable.h"
#include "taskdelegate.h"
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QDebug>
#include <QKeyEvent>



class TodoTableHeader : public QHeaderView
{
public:
    TodoTableHeader(Qt::Orientation o, QWidget *parent = 0)
        : QHeaderView(o, parent)
    {
        setClickable(false);
        setMovable(false);
        setHighlightSections(false);
        setResizeMode(QHeaderView::Custom);
        setFocusPolicy(Qt::NoFocus);
    }

protected:

    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
    {
        QStyle::State state = QStyle::State_None;

        int textAlignment = model()->headerData(logicalIndex, orientation(),
                Qt::TextAlignmentRole).toInt();

        QStyleOptionHeader opt;

        opt.palette = palette();
        opt.state = QStyle::State_None | QStyle::State_Raised;
        if (orientation() == Qt::Horizontal)
            opt.state |= QStyle::State_Horizontal;
        if (isEnabled())
            opt.state |= QStyle::State_Enabled;

        opt.rect = rect;
        opt.section = logicalIndex;
        opt.state |= state;
        opt.textAlignment = Qt::Alignment(textAlignment);
        opt.iconAlignment = Qt::AlignVCenter;

        opt.text = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        opt.icon = qVariantValue<QIcon>(model()->headerData(logicalIndex, orientation(), Qt::DecorationRole));

//      style()->drawPrimitive(QStyle::PE_PanelHeader, &opt, painter, this);
        style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);
        opt.rect = style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);

        if (!opt.icon.isNull()){
            // draw icon
            QPixmap pixmap =
                opt.icon.pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize),
                        opt.state & QStyle::State_Enabled ? QIcon::Normal
                        : QIcon::Disabled);

            style()->drawItemPixmap(painter, opt.rect, opt.iconAlignment, pixmap);
        } else {
            // draw text
            style()->drawItemText(painter, opt.rect, opt.textAlignment, opt.palette,
                    opt.state & QStyle::State_Enabled == QStyle::State_Enabled,
                    opt.text, QPalette::ButtonText);
        }
    }
};

TodoTable::TodoTable(QWidget *parent)
    : QTableView(parent)
{
    TaskTableDelegate* delegate = new TaskTableDelegate(this);
    setItemDelegate(delegate);
    setSelectionBehavior(SelectRows);
    connect(this, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(showTask(const QModelIndex &)));

    /*
    connect(this, SIGNAL(clicked(const QModelIndex &, Qt::MouseButton, Qt::KeyboardModifiers)),
            this, SLOT( activateItemClick(const QModelIndex &, Qt::MouseButton, Qt::KeyboardModifiers)));
    connect(this, SIGNAL(pressed(const QModelIndex &, Qt::MouseButton, Qt::KeyboardModifiers)),
            this, SLOT( activateItemPress(const QModelIndex &, Qt::MouseButton, Qt::KeyboardModifiers)));
    connect(this, SIGNAL(keyPressed(const QModelIndex &, Qt::Key, Qt::KeyboardModifiers)),
            this, SLOT( activateItemKey(const QModelIndex &, Qt::Key, Qt::KeyboardModifiers)));
            */
    setEditTriggers(SelectedClicked | EditKeyPressed | AnyKeyPressed);
    verticalHeader()->hide();

    TodoTableHeader *h = new TodoTableHeader(Qt::Horizontal, this);
    setHorizontalHeader(h);
}

TodoTable::~TodoTable() {}

void TodoTable::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);
    emit currentItemChanged(current);
}

void TodoTable::showTask(const QModelIndex &i)
{
    if (taskModel()) {
        QTask t = taskModel()->task(i);
        emit taskActivated(t);
    }
}

void TodoTable::setModel( QAbstractItemModel * model )
{
    QTaskModel *tm = qobject_cast<QTaskModel *>(model);
    if (!tm)
        return;
    QTableView::setModel(model);

    // show only columns this app is interested in.
    int i;
    if (taskModel()) {
        for (i = 0; i < taskModel()->columnCount(); i++ ) {
            switch(i) {
                case QTaskModel::Completed:
                case QTaskModel::Priority:
                case QTaskModel::Description:
                    showColumn(i);
                    break;
                default:
                    hideColumn(i);
                    break;
            }
        }
    }

    /* not sure about this bit.  Would be happier if were sure first number was a column reference, not a view reference */
    horizontalHeader()->moveSection(2, 0);
    horizontalHeader()->moveSection(1, 2);

    updateColumnSizes();
}

void TodoTable::toggleTaskCompleted(const QModelIndex &i)
{
    bool current = model()->data(i, Qt::EditRole).toBool();
    model()->setData(i, QVariant(!current), Qt::EditRole);
}


QList<QTask> TodoTable::selectedTasks() const
{
    QList<QTask> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(taskModel()->task(i));
    }
    return res;
}

QList<QUniqueId> TodoTable::selectedTaskIds() const
{
    QList<QUniqueId> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    // selection model for a table will list each cell in the row
    foreach(QModelIndex i, list) {
        QUniqueId id = taskModel()->id(i);
        if (!res.contains(id))
            res.append(id);
    }
    return res;
}

void TodoTable::resizeEvent(QResizeEvent * e)
{
    QTableView::resizeEvent(e);
    updateColumnSizes();
}

void TodoTable::keyPressEvent(QKeyEvent *e)
{
    QPersistentModelIndex oldCurrent = currentIndex();

    if(EditingState == state()) {
        QTableView::keyPressEvent(e);
        return;
    }

    QItemSelectionModel * smodel = selectionModel();
    QModelIndex ci = smodel->currentIndex();
    if(!ci.isValid())  {
        // Nothing is yet selected. ***HACK*** (or part of it) to get selection working properly.
        QTableView::keyPressEvent(e);
        if ( (e->key() == Qt::Key_Down) && (model()->rowCount() > 0) ) {
            // Select first in the list.
            setCurrentIndex(model()->sibling(0,QTaskModel::Description,currentIndex()));
            selectionModel()->select(currentIndex(),QItemSelectionModel::SelectCurrent);
        } else if ( (e->key() == Qt::Key_Up) && (model()->rowCount() > 0) ) {
            // Select last in the list.
            setCurrentIndex(model()->sibling(model()->rowCount()-1,QTaskModel::Description,currentIndex()));
            selectionModel()->select(currentIndex(),QItemSelectionModel::SelectCurrent);
        }
        return;
    }

    if(Qt::Key_Left == e->key()) {
        setSelectionBehavior(SelectItems);
        QModelIndex newmi = ci.sibling(ci.row(), QTaskModel::Completed);
        setCurrentIndex(newmi);
        smodel->select(newmi, QItemSelectionModel::ClearAndSelect);
        return;
    } else if(Qt::Key_Select == e->key() &&
              QTaskModel::Completed == ci.column()) {
        toggleTaskCompleted(ci);
        setCurrentIndex(ci);
        smodel->select(ci, QItemSelectionModel::Select);
        return;
    } else if(Qt::Key_Down == e->key() ||
              Qt::Key_Up == e->key()) {
        // I think they just forgot to put anything here. Well, never mind, I've sorted it out
        // below...
    } else {
        setSelectionBehavior(SelectRows);
    }

    QTableView::keyPressEvent(e);

    // ***HACK*** to ensure that we have wrap-around
    if ( (oldCurrent.row() == 0) && (e->key() == Qt::Key_Up) && (model()->rowCount() > 1) ) {
        // At the beginning of the list, keying down.
        setCurrentIndex(model()->sibling(model()->rowCount()-1,currentIndex().column(),currentIndex()));
    } else if ( (oldCurrent.row() == model()->rowCount()-1) && (e->key() == Qt::Key_Down) && (model()->rowCount() > 1) ) {
        // At the end of the list, keying up.
        setCurrentIndex(model()->sibling(0,currentIndex().column(),currentIndex()));
    }

    // ***HACK*** to ensure that the current item is actually selected.
    switch ( e->key() ) {
    case Qt::Key_Up:
    case Qt::Key_Down:
        // The keyPressEvent in QAbstractItemView only sets the 'current' index, not the selection.
        if ( (oldCurrent != currentIndex()) && (currentIndex().isValid()) ) {
            selectionModel()->select(currentIndex(),QItemSelectionModel::SelectCurrent);
        }
        break;
    }
}

void TodoTable::updateColumnSizes()
{
    TaskTableDelegate *del = taskDelegate();

    if (del) {
        QStyleOptionViewItem o;
        o.font = font();
        int totalwidth = 0;
        // header doesn't remap columns... so have to do 1,2,0 manually
        int width;
        width = del->sizeHint(o, QTaskModel::Priority).width();
        horizontalHeader()->resizeSection(QTaskModel::Priority, width);
        totalwidth += width;
        width = del->sizeHint(o, QTaskModel::Completed).width();
        horizontalHeader()->resizeSection(QTaskModel::Completed, width);
        totalwidth += width;

        width = horizontalHeader()->width() - totalwidth;
        horizontalHeader()->resizeSection(QTaskModel::Description, width);
    }
}
