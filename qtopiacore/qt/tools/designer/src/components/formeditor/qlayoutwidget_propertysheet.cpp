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

#include "qlayoutwidget_propertysheet.h"
#include "qlayout_widget_p.h"
#include "qdesigner_widget_p.h"
#include "formwindow.h"
#include "formeditor.h"

#include <QtDesigner/QExtensionManager>

#include <QLayout>
#include <QMetaObject>
#include <QMetaProperty>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

QLayoutWidgetPropertySheet::QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    m_fakeProperties.clear();
}

QLayoutWidgetPropertySheet::~QLayoutWidgetPropertySheet()
{
}

bool QLayoutWidgetPropertySheet::isVisible(int index) const
{
    QString name = propertyName(index);

    return name == QLatin1String("margin") || name == QLatin1String("spacing");
}

void QLayoutWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);

    QLayoutWidget *l = static_cast<QLayoutWidget*>(m_object);
    QDesignerFormEditorInterface *core = l->formWindow()->core();
    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), l->layout())) {
        sheet->setChanged(sheet->indexOf(propertyName(index)), true);
    }
}


QLayoutWidgetPropertySheetFactory::QLayoutWidgetPropertySheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QLayoutWidgetPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerPropertySheetExtension))
        return 0;

    if (QLayoutWidget *o = qobject_cast<QLayoutWidget*>(object))
        return new QLayoutWidgetPropertySheet(o, parent);

    return 0;
}
