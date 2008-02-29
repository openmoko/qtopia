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

#ifndef TOOLBAR_TASKMENU_H
#define TOOLBAR_TASKMENU_H

#include <QtGui/QToolBar>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <QtDesigner/default_extensionfactory.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ToolBarTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    ToolBarTaskMenu(QToolBar *button, QObject *parent = 0);
    virtual ~ToolBarTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editToolBar();

private:
    QToolBar *m_toolbar;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editTextAction;
};

class ToolBarTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    ToolBarTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // TOOLBAR_TASKMENU_H
