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

#ifndef QDESIGNER_WORKBENCH_H
#define QDESIGNER_WORKBENCH_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QRect>

class QDesigner;
class QDesignerActions;
class QDesignerToolWindow;
class QDesignerFormWindow;

class QAction;
class QActionGroup;
class QDockWidget;
class QMenu;
class QMenuBar;
class QVariant;
class QToolBar;
class QWorkspace;
class QCloseEvent;

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerFormWindowManagerInterface;

namespace qdesigner_internal {
class QDesignerIntegration;
}

class QDesignerWorkbench: public QObject
{
    Q_OBJECT
public:
    enum UIMode
    {
        NeutralMode,
        TopLevelMode,
        DockedMode
    };

public:
    QDesignerWorkbench();
    virtual ~QDesignerWorkbench();

    UIMode mode() const;

    QDesignerFormEditorInterface *core() const;

    QDesignerToolWindow *findToolWindow(QWidget *widget) const;
    QDesignerFormWindow *findFormWindow(QWidget *widget) const;

    QDesignerFormWindow *createFormWindow();

    int toolWindowCount() const;
    QDesignerToolWindow *toolWindow(int index) const;

    int formWindowCount() const;
    QDesignerFormWindow *formWindow(int index) const;

    QDesignerActions *actionManager() const;

    QActionGroup *modeActionGroup() const;

    QRect availableGeometry() const;
    int marginHint() const;

    void saveSettings() const;

    bool readInForm(const QString &fileName) const;
    bool writeOutForm(QDesignerFormWindowInterface *formWindow, const QString &fileName) const;
    bool saveForm(QDesignerFormWindowInterface *fw);
    bool handleClose();
    void closeAllToolWindows();

signals:
    void modeChanged(UIMode mode);
    void initialized();

public slots:
    void addToolWindow(QDesignerToolWindow *toolWindow);
    void addFormWindow(QDesignerFormWindow *formWindow);
    void removeToolWindow(QDesignerToolWindow *toolWindow);
    void removeFormWindow(QDesignerFormWindow *formWindow);
    void setUIMode(UIMode mode);

// ### private slots:
    void switchToNeutralMode();
    void switchToDockedMode();
    void switchToTopLevelMode();

    void initializeCorePlugins();

private slots:
    void initialize();
    void activateWorkspaceChildWindow(QWidget *widget);
    void updateWindowMenu(QDesignerFormWindowInterface *fw);
    void formWindowActionTriggered(QAction *a);
    void showToolBars();
    void adjustFormPositions();

private:
    QWidget *magicalParent() const;
    Qt::WindowFlags magicalWindowFlags(const QWidget *widgetForFlags) const;
    QDockWidget *magicalDockWidget(QWidget *widget) const;

    QDesignerFormWindowManagerInterface *formWindowManager() const;
    void changeBringToFrontVisiblity(bool visible);

    bool eventFilter(QObject *object, QEvent *event);

private:
    QDesignerFormEditorInterface *m_core;
    qdesigner_internal::QDesignerIntegration *m_integration;

    QDesignerActions *m_actionManager;
    QActionGroup *m_toolActions;
    QActionGroup *m_windowActions;

    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_formMenu;
    QMenu *m_toolMenu;
    QMenu *m_windowMenu;
    QMenu *m_helpMenu;

    QMenuBar *m_globalMenuBar;
    QToolBar *m_toolToolBar;
    QToolBar *m_formToolBar;
    QToolBar *m_editToolBar;

    UIMode m_mode;

    QList<QDesignerToolWindow*> m_toolWindows;
    QList<QDesignerFormWindow*> m_formWindows;

    QWorkspace *m_workspace;
    QHash<QWidget*, bool> m_visibilities;
    QHash<QWidget*, QRect> m_geometries;

    class ToolWindowExtra {};
    class FormWindowExtra {};

    QHash<QDesignerToolWindow*, ToolWindowExtra> m_toolWindowExtras;
    QHash<QDesignerFormWindow*, FormWindowExtra> m_formWindowExtras;
    bool m_initializing;
};

#endif // QDESIGNER_WORKBENCH_H
