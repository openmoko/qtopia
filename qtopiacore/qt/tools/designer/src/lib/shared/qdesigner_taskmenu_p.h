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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_TASKMENU_H
#define QDESIGNER_TASKMENU_H

#include "shared_global_p.h"
#include <QtDesigner/taskmenu.h>

#include <QtDesigner/default_extensionfactory.h>

#include <QtCore/QObject>
#include <QtCore/QPointer>

class QWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT QDesignerTaskMenu: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    QDesignerTaskMenu(QWidget *widget, QObject *parent);
    virtual ~QDesignerTaskMenu();

    QWidget *widget() const;

    virtual QList<QAction*> taskActions() const;

protected:
    QDesignerFormWindowInterface *formWindow() const;
    void changeRichTextProperty(const QString &propertyName);

private slots:
    void changeObjectName();
    void createDockWidget();
    void promoteToCustomWidget();
    void demoteFromCustomWidget();
    void changeToolTip();
    void changeStatusTip();
    void changeWhatsThis();
    void changeStyleSheet();
    void createMenuBar();
    void addToolBar();
    void createStatusBar();
    void removeStatusBar();

private:
    QPointer<QWidget> m_widget;
    QAction *m_separator;
    QAction *m_separator2;
    QAction *m_changeObjectNameAction;
    QAction *m_changeToolTip;
    QAction *m_changeStatusTip;
    QAction *m_changeWhatsThis;
    QAction *m_changeStyleSheet;
    QAction *m_createDockWidgetAction;
    QAction *m_promoteToCustomWidgetAction;
    QAction *m_demoteFromCustomWidgetAction;

    QAction *m_addMenuBar;
    QAction *m_addToolBar;
    QAction *m_addStatusBar;
    QAction *m_removeStatusBar;
};

class QDESIGNER_SHARED_EXPORT QDesignerTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    QDesignerTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_TASKMENU_H
