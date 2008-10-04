/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/


#include "qdesigner_propertyeditor.h"
#include "qdesigner_workbench.h"

#include <QtDesigner/QDesignerPropertyEditorInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerComponents>

#include <QtGui/QAction>

QT_BEGIN_NAMESPACE

QDesignerPropertyEditor::QDesignerPropertyEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("PropertyEditor"));
    QDesignerPropertyEditorInterface *widget = QDesignerComponents::createPropertyEditor(workbench->core(), this);
    workbench->core()->setPropertyEditor(widget);

    setCentralWidget(widget);

    setWindowTitle(tr("Property Editor"));
    action()->setShortcut(tr("Ctrl+I"));
    action()->setObjectName(QLatin1String("__qt_property_editor_action"));
}

QDesignerPropertyEditor::~QDesignerPropertyEditor()
{
}

QRect QDesignerPropertyEditor::geometryHint() const
{
    const QRect g = workbench()->availableGeometry();
    const int margin = workbench()->marginHint();
    const int spacing = 40;

    const QSize sz(g.width() * 1/4, g.height() * 4/6);

    return QRect((g.width() - sz.width() - margin), (margin + g.height() * 1/6) + spacing,
                  sz.width(), sz.height());
}

void QDesignerPropertyEditor::showEvent(QShowEvent *event)
{
    if (QDesignerPropertyEditorInterface *e = workbench()->core()->propertyEditor()) {
        // workaround to update the propertyeditor when it is not visible!
        e->setObject(e->object()); // ### remove me
    }

    QDesignerToolWindow::showEvent(event);
}

QT_END_NAMESPACE
