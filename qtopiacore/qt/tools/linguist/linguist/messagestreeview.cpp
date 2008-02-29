/****************************************************************************
**
** Copyright (C) 2006-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "messagestreeview.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QHeaderView>
#include <QtGui/QItemDelegate>

class MessagesItemDelegate : public QItemDelegate 
{
public:
    MessagesItemDelegate(QObject *parent) : QItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        const QAbstractItemModel *model = index.model();
        Q_ASSERT(model);

        if (!model->parent(index).isValid()) {
            if (index.column() == 1) {
                QStyleOptionViewItem opt = option;
                opt.font.setBold(true);
                QItemDelegate::paint(painter, opt, index);
                return;
            } 
        } 
        QItemDelegate::paint(painter, option, index);
    }
};

MessagesTreeView::MessagesTreeView(QWidget *parent) : QTreeView(parent)
{
    setRootIsDecorated(true);
    setItemsExpandable(true);
    setUniformRowHeights(true);
    setAlternatingRowColors(true);
    QPalette pal = palette();
    pal.setColor(QPalette::AlternateBase, TREEVIEW_ODD_COLOR);
    setPalette(pal);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setItemDelegate(new MessagesItemDelegate(this));
    header()->setSortIndicatorShown(true);
    header()->setClickable(true);
    header()->setMovable(false);
    setSortingEnabled(true);
}

void MessagesTreeView::setModel(QAbstractItemModel * model)
{
    QTreeView::setModel(model);
    QFontMetrics fm(font());
    header()->resizeSection(0, qMax(fm.width(tr("Done")), 64) );
    header()->setResizeMode(1, QHeaderView::Interactive);
    header()->setResizeMode(2, QHeaderView::Stretch);
    header()->setClickable(true);
}
