/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "tree_widget_p.h"

#include <QtCore/QPair>
#include <QtCore/QStack>

#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QScrollBar>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QStyle>

namespace qdesigner_internal {

class TreeWidgetDelegate: public QItemDelegate
{
public:
    TreeWidgetDelegate(TreeWidget *treeWidget)
        : QItemDelegate(treeWidget) {}


    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
    {
        QItemDelegate::paint(painter, option, index);

        QPen savedPen = painter->pen();
        QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
        painter->setPen(QPen(color));

        painter->drawLine(option.rect.x(), option.rect.bottom(),
                            option.rect.right(), option.rect.bottom());

        int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
        painter->drawLine(right, option.rect.y(), right, option.rect.bottom());

        painter->setPen(savedPen);
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QItemDelegate::sizeHint(option, index) + QSize(4,4);
    }
};


TreeWidget::TreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setItemDelegate(new TreeWidgetDelegate(this));

    setAlternatingRowColors(true);
}

TreeWidget::~TreeWidget()
{
}

static int level(QAbstractItemModel *model, const QModelIndex &index)
{
    int result = 0;
    QModelIndex parent = model->parent(index);
    while (parent.isValid()) {
        parent = model->parent(parent);
        ++result;
    }
    return result;
}

void TreeWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    // designer figts the style it uses. :(
    static bool mac_style 
        = QApplication::style()->inherits("QMacStyle");
    static const int windows_deco_size = 9;

    QStyleOptionViewItem option = viewOptions();

    if (model()->hasChildren(index)) {
        option.state |= QStyle::State_Children;

        const bool reverse = isRightToLeft();
        int indent = level(model(), index)*indentation();
        QRect primitive(reverse ? rect.left() : rect.left() + indent - 2,
                        rect.top(), indentation(), rect.height());

        if (!mac_style) {
            primitive.moveLeft(reverse ? primitive.left()
                               : primitive.left() + (primitive.width() - windows_deco_size)/2);
            primitive.moveTop(primitive.top() + (primitive.height() - windows_deco_size)/2);
            primitive.setWidth(windows_deco_size);
            primitive.setHeight(windows_deco_size);
        }

        option.rect = primitive;

        if (isExpanded(index))
            option.state |= QStyle::State_Open;

        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter, this);
    }
    QPen savedPen = painter->pen();
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    painter->setPen(QPen(color));
    painter->drawLine(rect.x(), rect.bottom(), rect.right(), rect.bottom());
    painter->setPen(savedPen);
}

} // namespace qdesigner_internal
