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

#include "q3table_extrainfo.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/private/ui4_p.h>

#include <Qt3Support/Q3Table>

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties) // ### remove me
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}

Q3TableExtraInfo::Q3TableExtraInfo(Q3Table *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{}

QWidget *Q3TableExtraInfo::widget() const
{ return m_widget; }

QDesignerFormEditorInterface *Q3TableExtraInfo::core() const
{ return m_core; }

bool Q3TableExtraInfo::saveUiExtraInfo(DomUi *ui)
{ Q_UNUSED(ui); return false; }

bool Q3TableExtraInfo::loadUiExtraInfo(DomUi *ui)
{ Q_UNUSED(ui); return false; }


bool Q3TableExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);

    Q3Table *table = qobject_cast<Q3Table*>(widget());
    Q_ASSERT(table != 0);
    Q_UNUSED(table);
    return true;
}

bool Q3TableExtraInfo::loadWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);

    Q3Table *table = qobject_cast<Q3Table*>(widget());
    Q_ASSERT(table != 0);
    Q_UNUSED(table);
    return true;
}

Q3TableExtraInfoFactory::Q3TableExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{}

QObject *Q3TableExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (Q3Table *w = qobject_cast<Q3Table*>(object))
        return new Q3TableExtraInfo(w, m_core, parent);

    return 0;
}
