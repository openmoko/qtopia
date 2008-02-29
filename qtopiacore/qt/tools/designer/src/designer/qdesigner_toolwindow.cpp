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

#include "qdesigner.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_settings.h"
#include "qdesigner_workbench.h"

#include <QtCore/QEvent>
#include <QtCore/qdebug.h>
#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QWorkspace>

QDesignerToolWindow::QDesignerToolWindow(QDesignerWorkbench *workbench, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_workbench(workbench),
      m_saveSettings(false)
{
    Q_ASSERT(workbench != 0);

    m_action = new QAction(this);
    m_action->setShortcutContext(Qt::ApplicationShortcut);
    m_action->setText(windowTitle());
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
}

QDesignerToolWindow::~QDesignerToolWindow()
{
    if (workbench())
        workbench()->removeToolWindow(this);
}

void QDesignerToolWindow::showMe(bool v)
{
    QWidget *target = parentWidget() == 0 ? this : parentWidget();

    if (v)
        target->setWindowState(target->windowState() & ~Qt::WindowMinimized);

    target->setVisible(v);
}

void QDesignerToolWindow::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);

    bool blocked = m_action->blockSignals(true);
    m_action->setChecked(true);
    m_action->blockSignals(blocked);
}

void QDesignerToolWindow::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);

    bool blocked = m_action->blockSignals(true);
    m_action->setChecked(false);
    m_action->blockSignals(blocked);
}

QAction *QDesignerToolWindow::action() const
{
    return m_action;
}

void QDesignerToolWindow::changeEvent(QEvent *e)
{
    switch (e->type()) {
        case QEvent::WindowTitleChange:
            m_action->setText(windowTitle());
            break;
        case QEvent::WindowIconChange:
            m_action->setIcon(windowIcon());
            break;
        default:
            break;
    }
    QMainWindow::changeEvent(e);
}

QDesignerWorkbench *QDesignerToolWindow::workbench() const
{
    return m_workbench;
}

QRect QDesignerToolWindow::geometryHint() const
{
    return QRect();
}

void QDesignerToolWindow::closeEvent(QCloseEvent *ev)
{
    if (m_saveSettings) {
        ev->setAccepted(workbench()->handleClose());
        if (ev->isAccepted() && qDesigner->mainWindow() == this)
            QMetaObject::invokeMethod(qDesigner, "quit", Qt::QueuedConnection);  // We're going down!
    } else {
        QMainWindow::closeEvent(ev);
    }
}

bool QDesignerToolWindow::saveSettingsOnClose() const
{
    return m_saveSettings;
}

void QDesignerToolWindow::setSaveSettingsOnClose(bool save)
{
    m_saveSettings = save;
}

Qt::DockWidgetArea QDesignerToolWindow::dockWidgetAreaHint() const
{
    return Qt::RightDockWidgetArea;
}

