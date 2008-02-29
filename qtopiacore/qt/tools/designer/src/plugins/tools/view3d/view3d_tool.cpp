/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QAction>
#include "view3d_tool.h"

QView3DTool::QView3DTool(QDesignerFormWindowInterface *formWindow, QObject *parent)
    :  QDesignerFormWindowToolInterface(parent)
{
    m_action = new QAction(tr("3DView"), this);
    m_formWindow = formWindow;
}

QDesignerFormEditorInterface *QView3DTool::core() const
{
    return m_formWindow->core();
}

QDesignerFormWindowInterface *QView3DTool::formWindow() const
{
    return m_formWindow;
}

QWidget *QView3DTool::editor() const
{
    if (m_editor == 0)
        m_editor = new QView3D(formWindow(), 0);

    return m_editor;
}

QAction *QView3DTool::action() const
{
    return m_action;
}

void QView3DTool::activated()
{
    if (m_editor != 0)
        m_editor->updateForm();
}

void QView3DTool::deactivated()
{
}

bool QView3DTool::handleEvent(QWidget*, QWidget*, QEvent*)
{
    return false;
}
