/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "actionrepository_p.h"

#include <QtGui/QDrag>
#include <QtGui/QContextMenuEvent>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

namespace qdesigner_internal {

ActionRepository::ActionRepository(QWidget *parent)
    : QListWidget(parent)
{
    setViewMode(IconMode);
    setMovement(Static);
    setResizeMode(Adjust);
    setIconSize(QSize(24, 24));
    setSpacing(iconSize().width() / 3);
    setTextElideMode(Qt::ElideRight);

    setDragEnabled(true);
    setAcceptDrops(false);
}

ActionRepository::~ActionRepository()
{
}

void ActionRepository::startDrag(Qt::DropActions supportedActions)
{
    if (!selectionModel())
        return;

    QModelIndexList indexes = selectionModel()->selectedIndexes();

    if (indexes.count() > 0) {
        QDrag *drag = new QDrag(this);
        QIcon icon = qvariant_cast<QIcon>(model()->data(indexes.front(), Qt::DecorationRole));
        drag->setPixmap(icon.pixmap(QSize(22, 22)));
        drag->setMimeData(model()->mimeData(indexes));
        drag->start(supportedActions);
    }
}

QMimeData *ActionRepository::mimeData(const QList<QListWidgetItem*> items) const
{
    ActionRepositoryMimeData *data = new ActionRepositoryMimeData();
    foreach (QListWidgetItem *item, items) {
        QAction *action = qvariant_cast<QAction*>(item->data(ActionRole));
        data->items.append(action);
    }
    return data;
}

void ActionRepository::filter(const QString &text)
{
    QSet<QListWidgetItem*> visibleItems = QSet<QListWidgetItem*>::fromList(findItems(text, Qt::MatchContains));
    for (int index=0; index<count(); ++index) {
        QListWidgetItem *i = item(index);
        setItemHidden(i, !visibleItems.contains(i));
    }
}

void ActionRepository::focusInEvent(QFocusEvent *event)
{
    QListWidget::focusInEvent(event);
    if (currentItem()) {
        emit currentItemChanged(currentItem(), currentItem());
    }
}

void ActionRepository::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenuRequested(event, itemAt(event->pos()));
}

} // namespace qdesigner_internal
