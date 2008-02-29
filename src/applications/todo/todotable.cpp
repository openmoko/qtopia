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

#include "todotable.h"
#include "taskdelegate.h"
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QDebug>
#include <QKeyEvent>
#include "qsoftmenubar.h"

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
    connect(delegate, SIGNAL(showItem(const QModelIndex &)),
            this, SLOT(showTask(const QModelIndex &)));
    connect(delegate, SIGNAL(toggleItemCompleted(const QModelIndex &)),
            this, SLOT(toggleTaskCompleted(const QModelIndex &)));
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
    selectionModel()->select(current, QItemSelectionModel::Select);
    /* and save the uid, so we can reselect this if we change things */
    if ( current.isValid() ) {
        lastSelectedTaskId = taskModel()->id(current);
    } else {
        lastSelectedTaskId = QUniqueId();
    }
    switch(current.column()) {
        case QTaskModel::Priority:
        case QTaskModel::Completed:
            /* Make it clear Select does something */
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select);
            break;

        default:
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::View);
            break;
    }
    emit currentItemChanged(current);
}

void TodoTable::reset()
{
    /* base class stuff */
    QTableView::reset();

    /* Make sure we're back in row mode */
    setSelectionBehavior(SelectRows);

    /* Select the last selected task, if any, otherwise the first task, if any  */
    if ( !currentIndex().isValid() ) {
        QModelIndex newSel = taskModel()->index(lastSelectedTaskId);
        if ( !newSel.isValid() ) {
            newSel = taskModel()->index(0,0);
        }
        if( newSel.isValid() )
            selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    } else {
        /* Make sure it is a whole row */
        selectionModel()->setCurrentIndex(currentIndex(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
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
    // Make sure this task is the current one.
    setCurrentIndex(i);

    // And toggle it
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

    switch ( e->key() ) {
        case Qt::Key_Left:
            if (selectionBehavior() != SelectItems) {
                setSelectionBehavior(SelectItems);
                QModelIndex newmi = ci.sibling(ci.row(), QTaskModel::Priority);
                setCurrentIndex(newmi);
                smodel->select(newmi, QItemSelectionModel::ClearAndSelect);
                return;
            }
            break;

        case Qt::Key_Right:
            if (selectionBehavior() == SelectItems && QTaskModel::Priority== ci.column()) {
                setSelectionBehavior(SelectRows);
                QModelIndex newmi = ci.sibling(ci.row(), QTaskModel::Description);
                setCurrentIndex(newmi);
                smodel->select(newmi, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                return;
            }
            break;

        case Qt::Key_Select:
            if(QTaskModel::Completed == ci.column()) {
                toggleTaskCompleted(ci);
                return;
            } else if (QTaskModel::Description == ci.column()) {
                showTask(ci);
                return;
            }
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
            /* Ignore these here, they get handled in the base class */
            break;

        default:
            setSelectionBehavior(SelectRows);
            QModelIndex newmi = ci.sibling(ci.row(), QTaskModel::Description);
            setCurrentIndex(newmi);
            smodel->select(newmi, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }

    QTableView::keyPressEvent(e);

    // work around to ensure that we have wrap-around (since QTableView doesn't wrap)
    if (model()->rowCount() > 1) {
        if ( (oldCurrent.row() == 0) && (e->key() == Qt::Key_Up) ) {
            // At the beginning of the list, keying down.
            setCurrentIndex(model()->sibling(model()->rowCount()-1,currentIndex().column(),currentIndex()));
        } else if ( (oldCurrent.row() == model()->rowCount()-1) && (e->key() == Qt::Key_Down) ) {
            // At the end of the list, keying up.
            setCurrentIndex(model()->sibling(0,currentIndex().column(),currentIndex()));
        }
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
