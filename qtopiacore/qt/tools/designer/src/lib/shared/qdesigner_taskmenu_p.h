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
