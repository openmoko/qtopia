/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
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

    if (name == QLatin1String("frameShape"))
        return false;
    return QDesignerPropertySheet::isVisible(index);
}

void LinePropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);
}

QString LinePropertySheet::propertyGroup(int index) const
{
    return QDesignerPropertySheet::propertyGroup(index);
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
