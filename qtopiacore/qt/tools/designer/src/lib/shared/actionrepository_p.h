/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
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

#ifndef ACTIONREPOSITORY_H
#define ACTIONREPOSITORY_H

#include "shared_global_p.h"
#include <QtCore/QMimeData>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTreeView>
#include <QtGui/QListView>
#include <QtGui/QStackedWidget>

QT_BEGIN_NAMESPACE

class QPixmap;

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

// Shared model of actions, to be used for several views (detailed/icon view).
class QDESIGNER_SHARED_EXPORT ActionModel: public QStandardItemModel
{
    Q_OBJECT
public:
    enum Columns { NameColumn, UsedColumn, TextColumn, ShortCutColumn, CheckedColumn, ToolTipColumn, NumColumns };
    enum   { ActionRole = Qt::UserRole + 1000 };

    explicit ActionModel(QWidget *parent = 0);
    void initialize(QDesignerFormEditorInterface *core) { m_core = core; }

    void clearActions();
    QModelIndex addAction(QAction *a);
    // remove row
    void remove(int row);
    // update the row from the underlying action
    void update(int row);

    // return row of action or -1.
    int findAction(QAction *) const;

    QString actionName(int row) const;
    QAction *actionAt(const QModelIndex &index) const;

    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    // Find the associated menus and toolbars, ignore toolbuttons
    static QWidgetList associatedWidgets(const QAction *action);

signals:
    void resourceImageDropped(const QString &path, QAction *action);

private:
    void initializeHeaders();

    typedef QList<QStandardItem *> QStandardItemList;
    static void setItems(QDesignerFormEditorInterface *core, QAction *a, QStandardItemList &sl);

    QDesignerFormEditorInterface *m_core;
};

// Internal class that provides the detailed view of actions.
class  ActionTreeView: public QTreeView
{
    Q_OBJECT
public:
    explicit ActionTreeView(ActionModel *model, QWidget *parent = 0);
    QAction *currentAction() const;

public slots:
    void filter(const QString &text);

signals:
    void contextMenuRequested(QContextMenuEvent *event, QAction *);
    void currentChanged(QAction *action);
    void activated(QAction *action);

protected slots:
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void startDrag(Qt::DropActions supportedActions);

private slots:
    void slotActivated(const QModelIndex &);

private:
    ActionModel *m_model;
};

// Internal class that provides the icon view of actions.
class ActionListView: public QListView
{
    Q_OBJECT
public:
    explicit ActionListView(ActionModel *model, QWidget *parent = 0);
    QAction *currentAction() const;

public slots:
    void filter(const QString &text);

signals:
    void contextMenuRequested(QContextMenuEvent *event, QAction *);
    void currentChanged(QAction *action);
    void activated(QAction *action);

protected slots:
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void startDrag(Qt::DropActions supportedActions);

private slots:
    void slotActivated(const QModelIndex &);

private:
    ActionModel *m_model;
};

// Action View that can be switched between detailed and icon view
// using a  QStackedWidget of  ActionListView / ActionTreeView
// that share the item model and the selection model.

class ActionView : public  QStackedWidget {
    Q_OBJECT
public:
    // Separate initialize() function takes core argument to make this
    // thing usable as promoted widget.
    explicit ActionView(QWidget *parent = 0);
    void initialize(QDesignerFormEditorInterface *core) { m_model->initialize(core); }

    // View mode
    enum { DetailedView, IconView };
    int viewMode() const;
    void setViewMode(int lm);

    void setSelectionMode(QAbstractItemView::SelectionMode sm);
    QAbstractItemView::SelectionMode selectionMode() const;

    ActionModel *model() const { return m_model; }

    QAction *currentAction() const;
    void setCurrentIndex(const QModelIndex &index);

    typedef QList<QAction*> ActionList;
    ActionList selectedActions() const;
    QItemSelection selection() const;

public slots:
    void filter(const QString &text);
    void selectAll();
    void clearSelection();

signals:
    void contextMenuRequested(QContextMenuEvent *event, QAction *);
    void currentChanged(QAction *action);
    void activated(QAction *action);
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void resourceImageDropped(const QString &data, QAction *action);

private slots:
    void slotCurrentChanged(QAction *action);

private:
    ActionModel *m_model;
    ActionTreeView *m_actionTreeView;
    ActionListView *m_actionListView;
};

class QDESIGNER_SHARED_EXPORT ActionRepositoryMimeData: public QMimeData
{
    Q_OBJECT
public:
    typedef QList<QAction*> ActionList;

    ActionRepositoryMimeData(const ActionList &, Qt::DropAction dropAction);
    ActionRepositoryMimeData(QAction *, Qt::DropAction dropAction);

    const ActionList &actionList() const { return m_actionList; }
    virtual QStringList formats() const;

    static QPixmap actionDragPixmap(const QAction *action);

    // Utility to accept with right action
    void accept(QDragMoveEvent *event) const;
private:
    const Qt::DropAction m_dropAction;
    ActionList m_actionList;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ACTIONREPOSITORY_H
