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

#include "qdesigner_dnditem_p.h"

#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/QLabel>
#include <QtCore/qdebug.h>

#include "ui4_p.h"

namespace qdesigner_internal {

QDesignerDnDItem::QDesignerDnDItem(DropType type, QWidget *source)
{
    m_source = source;
    m_type = type;
    m_dom_ui = 0;
    m_widget = 0;
    m_decoration = 0;
}

void QDesignerDnDItem::init(DomUI *ui, QWidget *widget, QWidget *decoration,
                                    const QPoint &global_mouse_pos)
{
    Q_ASSERT(widget != 0 || ui != 0);
    Q_ASSERT(decoration != 0);

    m_dom_ui = ui;
    m_widget = widget;
    m_decoration = decoration;

    QRect geometry = m_decoration->geometry();
    m_hot_spot = global_mouse_pos - m_decoration->geometry().topLeft();
}

QDesignerDnDItem::~QDesignerDnDItem()
{
    if (m_decoration != 0)
        m_decoration->deleteLater();
    delete m_dom_ui;
}

DomUI *QDesignerDnDItem::domUi() const
{
    return m_dom_ui;
}

QWidget *QDesignerDnDItem::decoration() const
{
    return m_decoration;
}

QPoint QDesignerDnDItem::hotSpot() const
{
    return m_hot_spot;
}

QWidget *QDesignerDnDItem::widget() const
{
    return m_widget;
}

QDesignerDnDItem::DropType QDesignerDnDItem::type() const
{
    return m_type;
}

QWidget *QDesignerDnDItem::source() const
{
    return m_source;
}

void QDesignerDnDItem::setDomUi(DomUI *dom_ui)
{
    delete m_dom_ui;
    m_dom_ui = dom_ui;
}


} // namespace qdesigner_internal
