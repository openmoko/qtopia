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

#ifndef CONTAINERWIDGER_TASKMENU_H
#define CONTAINERWIDGER_TASKMENU_H

#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <QtDesigner/default_extensionfactory.h>

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QDesignerContainerExtension;
class QAction;

namespace qdesigner_internal {

class ContainerWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    ContainerWidgetTaskMenu(QWidget *widget, QObject *parent = 0);
    virtual ~ContainerWidgetTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();

private:
    QDesignerFormEditorInterface *core() const;
    QDesignerFormWindowInterface *formWindow() const;
    QDesignerContainerExtension *containterExtension() const;

private:
    QWidget *m_containerWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QList<QAction*> m_taskActions;

    QAction *m_actionPreviousPage;
    QAction *m_actionNextPage;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
};

class ContainerWidgetTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    ContainerWidgetTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // CONTAINERWIDGER_TASKMENU_H
