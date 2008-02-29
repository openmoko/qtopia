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
