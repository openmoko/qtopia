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

#include <QtGui/QLabel>
#include <QtGui/QPixmap>

#include "ui4_p.h"
#include <qdesigner_resource.h>

#include "formwindow_dnditem.h"
#include "formwindow.h"

using namespace qdesigner_internal;

static QWidget *decorationFromWidget(QWidget *w)
{
    QLabel *label = new QLabel(0, Qt::ToolTip);
    QPixmap pm = QPixmap::grabWidget(w);
    label->setPixmap(pm);
    label->resize(pm.size());

    return label;
}

static DomUI *widgetToDom(QWidget *widget, FormWindow *form)
{
    QDesignerResource builder(form);
    return builder.copy(QList<QWidget*>() << widget);
}

FormWindowDnDItem::FormWindowDnDItem(QDesignerDnDItemInterface::DropType type, FormWindow *form,
                                        QWidget *widget, const QPoint &global_mouse_pos)
    : QDesignerDnDItem(type, form)
{
    QWidget *decoration = decorationFromWidget(widget);
    QPoint pos = widget->mapToGlobal(QPoint(0, 0));
    decoration->move(pos);

    init(0, widget, decoration, global_mouse_pos);
}

DomUI *FormWindowDnDItem::domUi() const
{
    DomUI *result = QDesignerDnDItem::domUi();
    if (result != 0)
        return result;
    FormWindow *form = qobject_cast<FormWindow*>(source());
    if (widget() == 0 || form == 0)
        return 0;

    result = widgetToDom(widget(), form);
    const_cast<FormWindowDnDItem*>(this)->setDomUi(result);
    return result;
}


