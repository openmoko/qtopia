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

#include "qdesigner.h"
#include "qdesigner_actioneditor.h"
#include "qdesigner_workbench.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QDesignerComponents>

QDesignerActionEditor::QDesignerActionEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("ActionEditor"));
    QDesignerActionEditorInterface *widget = QDesignerComponents::createActionEditor(workbench->core(), this);
    workbench->core()->setActionEditor(widget);

    setCentralWidget(widget);

    setWindowTitle(tr("Action Editor"));
}

QDesignerActionEditor::~QDesignerActionEditor()
{
}

QRect QDesignerActionEditor::geometryHint() const
{
    QRect g = workbench()->availableGeometry();
    int margin = workbench()->marginHint();

    QSize sz(g.width() * 1/4, g.height() * 1/6);

    return QRect((g.width() - sz.width() - margin), margin,
                  sz.width(), sz.height());
}
