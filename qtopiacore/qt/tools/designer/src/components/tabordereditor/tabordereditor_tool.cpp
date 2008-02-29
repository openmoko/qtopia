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

/*
TRANSLATOR qdesigner_internal::TabOrderEditorTool
*/

#include "tabordereditor_tool.h"
#include "tabordereditor.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QEvent>
#include <QtGui/QAction>

using namespace qdesigner_internal;

TabOrderEditorTool::TabOrderEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent)
    : QDesignerFormWindowToolInterface(parent),
      m_formWindow(formWindow)
{
    m_action = new QAction(tr("Edit Tab Order"), this);
}

TabOrderEditorTool::~TabOrderEditorTool()
{
}

QDesignerFormEditorInterface *TabOrderEditorTool::core() const
{
    return m_formWindow->core();
}

QDesignerFormWindowInterface *TabOrderEditorTool::formWindow() const
{
    return m_formWindow;
}

bool TabOrderEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);

    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        return true;

    return false;
}

QWidget *TabOrderEditorTool::editor() const
{
    if (!m_editor) {
        Q_ASSERT(formWindow() != 0);
        m_editor = new TabOrderEditor(formWindow(), 0);
        connect(formWindow(), SIGNAL(mainContainerChanged(QWidget*)), m_editor, SLOT(setBackground(QWidget*)));
    }

    return m_editor;
}

void TabOrderEditorTool::activated()
{
    connect(formWindow(), SIGNAL(changed()),
                m_editor, SLOT(updateBackground()));
}

void TabOrderEditorTool::deactivated()
{
    disconnect(formWindow(), SIGNAL(changed()),
                m_editor, SLOT(updateBackground()));
}

QAction *TabOrderEditorTool::action() const
{
    return m_action;
}

