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

#include "default_layoutdecoration.h"
#include "formwindow.h"
#include "qlayout_widget_p.h"
#include "qdesigner_widget_p.h"

#include <QtGui/QGridLayout>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

// ---- QDesignerLayoutDecoration ----
QDesignerLayoutDecoration::QDesignerLayoutDecoration(QLayoutWidget *widget, QObject *parent)
    : QObject(parent),
      m_layoutSupport(widget->support())
{
    Q_ASSERT(m_layoutSupport);
}

QDesignerLayoutDecoration::QDesignerLayoutDecoration(FormWindow *formWindow, QWidget *widget, QObject *parent)
    : QObject(parent),
      m_layoutSupport(new QLayoutSupport(formWindow, widget, this))
{
    Q_ASSERT(m_layoutSupport);
}

QDesignerLayoutDecoration::~QDesignerLayoutDecoration()
{
}

QList<QWidget*> QDesignerLayoutDecoration::widgets(QLayout *layout) const
{
    return m_layoutSupport->widgets(layout);
}

QRect QDesignerLayoutDecoration::itemInfo(int index) const
{
    return m_layoutSupport->itemInfo(index);
}

int QDesignerLayoutDecoration::indexOf(QWidget *widget) const
{
    return m_layoutSupport->indexOf(widget);
}

int QDesignerLayoutDecoration::indexOf(QLayoutItem *item) const
{
    return m_layoutSupport->indexOf(item);
}

QDesignerLayoutDecoration::InsertMode QDesignerLayoutDecoration::currentInsertMode() const
{
    return m_layoutSupport->currentInsertMode();
}

void QDesignerLayoutDecoration::insertWidget(QWidget *widget, const QPair<int, int> &cell)
{
    m_layoutSupport->insertWidget(widget, cell);
}

void QDesignerLayoutDecoration::removeWidget(QWidget *widget)
{
    m_layoutSupport->removeWidget(widget);
}

void QDesignerLayoutDecoration::insertRow(int row)
{
    m_layoutSupport->insertRow(row);
}

void QDesignerLayoutDecoration::insertColumn(int column)
{
    m_layoutSupport->insertColumn(column);
}

void QDesignerLayoutDecoration::simplify()
{
    m_layoutSupport->simplifyLayout();
}

int QDesignerLayoutDecoration::currentIndex() const
{
    return m_layoutSupport->currentIndex();
}

QPair<int, int> QDesignerLayoutDecoration::currentCell() const
{
    return m_layoutSupport->currentCell();
}

int QDesignerLayoutDecoration::findItemAt(const QPoint &pos) const
{
    return m_layoutSupport->findItemAt(pos);
}

int QDesignerLayoutDecoration::findItemAt(int row, int column) const
{
    return m_layoutSupport->findItemAt(row, column);
}

void QDesignerLayoutDecoration::adjustIndicator(const QPoint &pos, int index)
{
    m_layoutSupport->adjustIndicator(pos, index);
}

// ---- QDesignerLayoutDecorationFactory ----
QDesignerLayoutDecorationFactory::QDesignerLayoutDecorationFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerLayoutDecorationFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerLayoutDecorationExtension))
        return 0;

    if (QLayoutWidget *widget = qobject_cast<QLayoutWidget*>(object)) {
        return new QDesignerLayoutDecoration(widget, parent);
    } else if (QWidget *widget = qobject_cast<QWidget*>(object)) {
        if (FormWindow *fw = FormWindow::findFormWindow(widget)) {
            QDesignerMetaDataBaseItemInterface *item = fw->core()->metaDataBase()->item(widget->layout());
            return item ? new QDesignerLayoutDecoration(fw, widget, parent) : 0;
        }
    }

    return 0;
}
