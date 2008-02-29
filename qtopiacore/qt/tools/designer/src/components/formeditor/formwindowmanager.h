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

#ifndef FORMWINDOWMANAGER_H
#define FORMWINDOWMANAGER_H

#include "formeditor_global.h"
#include "formwindow.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPointer>

class QAction;
class QActionGroup;
class QUndoGroup;
class MetaDataBase;
class QDesignerFormEditorInterface;
class QDesignerWidgetBoxInterface;

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormWindowManager: public QDesignerFormWindowManagerInterface
{
    Q_OBJECT
public:
    FormWindowManager(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~FormWindowManager();

    virtual QDesignerFormEditorInterface *core() const;

    inline QAction *actionCut() const { return m_actionCut; }
    inline QAction *actionCopy() const { return m_actionCopy; }
    inline QAction *actionPaste() const { return m_actionPaste; }
    inline QAction *actionDelete() const { return m_actionDelete; }
    inline QAction *actionSelectAll() const { return m_actionSelectAll; }
    inline QAction *actionLower() const { return m_actionLower; }
    inline QAction *actionRaise() const { return m_actionRaise; }
    QAction *actionUndo() const;
    QAction *actionRedo() const;

    inline QAction *actionHorizontalLayout() const { return m_actionHorizontalLayout; }
    inline QAction *actionVerticalLayout() const { return m_actionVerticalLayout; }
    inline QAction *actionSplitHorizontal() const { return m_actionSplitHorizontal; }
    inline QAction *actionSplitVertical() const { return m_actionSplitVertical; }
    inline QAction *actionGridLayout() const { return m_actionGridLayout; }
    inline QAction *actionBreakLayout() const { return m_actionBreakLayout; }
    inline QAction *actionAdjustSize() const { return m_actionAdjustSize; }

    QDesignerFormWindowInterface *activeFormWindow() const;

    int formWindowCount() const;
    QDesignerFormWindowInterface *formWindow(int index) const;

    QDesignerFormWindowInterface *createFormWindow(QWidget *parentWidget = 0, Qt::WindowFlags flags = 0);

    bool eventFilter(QObject *o, QEvent *e);

    void dragItems(const QList<QDesignerDnDItemInterface*> &item_list);

    QUndoGroup *undoGroup() const;

public slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow);
    void removeFormWindow(QDesignerFormWindowInterface *formWindow);
    void setActiveFormWindow(QDesignerFormWindowInterface *formWindow);

private slots:
    void slotActionCutActivated();
    void slotActionCopyActivated();
    void slotActionPasteActivated();
    void slotActionDeleteActivated();
    void slotActionSelectAllActivated();
    void slotActionLowerActivated();
    void slotActionRaiseActivated();
    void slotActionHorizontalLayoutActivated();
    void slotActionVerticalLayoutActivated();
    void slotActionSplitHorizontalActivated();
    void slotActionSplitVerticalActivated();
    void slotActionGridLayoutActivated();
    void slotActionBreakLayoutActivated();
    void slotActionAdjustSizeActivated();

    void slotUpdateActions();

private:
    void setupActions();
    FormWindow *findFormWindow(QWidget *w);
    QWidget *findManagedWidget(FormWindow *fw, QWidget *w);

    void layoutContainerHorizontal();
    void layoutContainerVertical();
    void layoutContainerGrid();

    void setCurrentUndoStack(QUndoStack *stack);

private:
    QDesignerFormEditorInterface *m_core;
    FormWindow *m_activeFormWindow;
    QList<FormWindow*> m_formWindows;

    bool m_layoutChilds;

    // edit actions
    QAction *m_actionCut;
    QAction *m_actionCopy;
    QAction *m_actionPaste;
    QAction *m_actionSelectAll;
    QAction *m_actionDelete;
    QAction *m_actionLower;
    QAction *m_actionRaise;
    // layout actions
    QAction *m_actionHorizontalLayout;
    QAction *m_actionVerticalLayout;
    QAction *m_actionSplitHorizontal;
    QAction *m_actionSplitVertical;
    QAction *m_actionGridLayout;
    QAction *m_actionBreakLayout;
    QAction *m_actionAdjustSize;

    QAction *m_actionUndo;
    QAction *m_actionRedo;

    // DnD stuff
    void beginDrag(const QList<QDesignerDnDItemInterface*> &item_list, const QPoint &globalPos);
    void endDrag(const QPoint &pos);
    void setItemsPos(const QPoint &pos);
    bool isDecoration(QWidget *widget) const;
    QList<QDesignerDnDItemInterface*> m_drag_item_list;
    QWidget *m_last_widget_under_mouse;
    FormWindow *m_last_form_under_mouse;
    QDesignerWidgetBoxInterface *m_widget_box_under_mouse;
    Qt::ContextMenuPolicy m_savedContextMenuPolicy;
    QUndoGroup *m_undoGroup;
};

}  // namespace qdesigner_internal

#endif // FORMWINDOWMANAGER_H
