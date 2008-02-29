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

/*
TRANSLATOR qdesigner_internal::ContainerWidgetTaskMenu
*/

#include "containerwidget_taskmenu.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>

#include <qdesigner_command_p.h>
#include <qdesigner_stackedbox_p.h>
#include <qdesigner_tabwidget_p.h>
#include <qdesigner_toolbox_p.h>
#include <qdesigner_dockwidget_p.h>

#include <QtGui/QAction>
#include <QtGui/QMainWindow>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

ContainerWidgetTaskMenu::ContainerWidgetTaskMenu(QWidget *widget, QObject *parent)
    : QDesignerTaskMenu(widget, parent),
      m_containerWidget(widget)
{
    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);

    m_actionDeletePage = new QAction(tr("Delete Page"), this);
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));

    m_actionInsertPage = new QAction(tr("Insert Page Before Current Page"), this);
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));

    m_actionInsertPageAfter = new QAction(tr("Insert Page After Current Page"), this);
    connect(m_actionInsertPageAfter, SIGNAL(triggered()), this, SLOT(addPageAfter()));

    m_taskActions.append(m_actionDeletePage);

    sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);

    m_taskActions.append(m_actionInsertPageAfter);
    m_taskActions.append(m_actionInsertPage);
}

ContainerWidgetTaskMenu::~ContainerWidgetTaskMenu()
{
}

QAction *ContainerWidgetTaskMenu::preferredEditAction() const
{
    return 0;
}

QList<QAction*> ContainerWidgetTaskMenu::taskActions() const
{
    QList<QAction*> actions = QDesignerTaskMenu::taskActions();
    actions += m_taskActions;
    if (QDesignerContainerExtension *ce = containterExtension())
        m_actionDeletePage->setEnabled(ce->count() > 1);
    return actions;
}

QDesignerFormEditorInterface *ContainerWidgetTaskMenu::core() const
{
    if (QDesignerFormWindowInterface *fw = formWindow())
        return fw->core();

    return 0;
}

QDesignerFormWindowInterface *ContainerWidgetTaskMenu::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(m_containerWidget);
}

QDesignerContainerExtension *ContainerWidgetTaskMenu::containterExtension() const
{
    if (QDesignerFormEditorInterface *ed = core()) {
        QExtensionManager *mgr = ed->extensionManager();
        return qt_extension<QDesignerContainerExtension*>(mgr, m_containerWidget);
    }

    return 0;
}

void ContainerWidgetTaskMenu::removeCurrentPage()
{
    if (QDesignerContainerExtension *c = containterExtension()) {
        if (c->currentIndex() == -1)
            return;

        QDesignerFormWindowInterface *fw = formWindow();
        DeleteContainerWidgetPageCommand *cmd = new DeleteContainerWidgetPageCommand(fw);
        cmd->init(m_containerWidget);
        fw->commandHistory()->push(cmd);
    }
}

void ContainerWidgetTaskMenu::addPage()
{
    if (containterExtension()) {
        QDesignerFormWindowInterface *fw = formWindow();
        AddContainerWidgetPageCommand *cmd = new AddContainerWidgetPageCommand(fw);
        cmd->init(m_containerWidget, AddContainerWidgetPageCommand::InsertBefore);
        fw->commandHistory()->push(cmd);
    }
}

void ContainerWidgetTaskMenu::addPageAfter()
{
    if (containterExtension()) {
        QDesignerFormWindowInterface *fw = formWindow();
        AddContainerWidgetPageCommand *cmd = new AddContainerWidgetPageCommand(fw);
        cmd->init(m_containerWidget, AddContainerWidgetPageCommand::InsertAfter);
        fw->commandHistory()->push(cmd);
    }
}

ContainerWidgetTaskMenuFactory::ContainerWidgetTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *ContainerWidgetTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
        return 0;

    QWidget *widget = qobject_cast<QWidget*>(object);
    if (!widget)
        return 0;

    if (qobject_cast<QDesignerStackedWidget*>(widget)
            || qobject_cast<QDesignerToolBox*>(widget)
            || qobject_cast<QDesignerTabWidget*>(widget)
            || qobject_cast<QDesignerDockWidget*>(widget)
            || qobject_cast<QMainWindow*>(widget))
        return 0;

    if (qt_extension<QDesignerContainerExtension*>(extensionManager(), object)) {
        return new ContainerWidgetTaskMenu(widget, parent);
    }

    return 0;
}
