/****************************************************************************
**
** Copyright (C) 2006-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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
