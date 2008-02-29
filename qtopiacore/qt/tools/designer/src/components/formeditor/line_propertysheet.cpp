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

#include "line_propertysheet.h"
#include "formwindow.h"

// sdk
#include <QtDesigner/QExtensionManager>

// shared
#include <qdesigner_widget_p.h>


#include <QtGui/QLayout>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

LinePropertySheet::LinePropertySheet(Line *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    m_fakeProperties.clear();
}

LinePropertySheet::~LinePropertySheet()
{
}

bool LinePropertySheet::isVisible(int index) const
{
    QString name = propertyName(index);

    return (name == QLatin1String("geometry")
            || name == QLatin1String("orientation")
            || name == QLatin1String("objectName"));
}

void LinePropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);
}

QString LinePropertySheet::propertyGroup(int index) const
{
    QString g = QDesignerPropertySheet::propertyGroup(index);
    if (g == QLatin1String("QObject"))
        return g;

    return QLatin1String("Line");
}

LinePropertySheetFactory::LinePropertySheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *LinePropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerPropertySheetExtension))
        return 0;

    if (Line *o = qobject_cast<Line*>(object))
        return new LinePropertySheet(o, parent);

    return 0;
}
