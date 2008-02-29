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

#ifndef QDESIGNER_TOOLBAR_H
#define QDESIGNER_TOOLBAR_H

#include "shared_global_p.h"

#include <QtGui/QAction>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>

class QTimer;
class QDesignerFormWindowInterface;
class QDesignerActionProviderExtension;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT SentinelAction: public QAction
{
    Q_OBJECT
public:
    SentinelAction(QWidget *widget);
    virtual ~SentinelAction();
};

class QDESIGNER_SHARED_EXPORT Sentinel: public QToolButton
{
    Q_OBJECT
public:
    Sentinel(QWidget *widget);
    virtual ~Sentinel();
};

} // namespace qdesigner_internal

class QDESIGNER_SHARED_EXPORT QDesignerToolBar: public QToolBar
{
    Q_OBJECT
public:
    QDesignerToolBar(QWidget *parent = 0);
    virtual ~QDesignerToolBar();

    bool eventFilter(QObject *object, QEvent *event);

    bool interactive(bool i);
    void adjustSpecialActions();

    QDesignerFormWindowInterface *formWindow() const;
    QDesignerActionProviderExtension *actionProvider();

private slots:
    void slotRemoveSelectedAction();
    void slotNewToolBar();
    void slotRemoveToolBar();
    void slotInsertSeparator();

protected:
    virtual void actionEvent(QActionEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);

    void startDrag(const QPoint &pos);
    bool handleEvent(QWidget *widget, QEvent *event);
    bool handleMousePressEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseReleaseEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseMoveEvent(QWidget *widget, QMouseEvent *event);
    bool handleContextMenuEvent(QWidget *widget, QContextMenuEvent *event);

    void adjustIndicator(const QPoint &pos);
    int findAction(const QPoint &pos) const;
    bool isPassiveWidget(QWidget *widget) const;
    QAction *createAction(const QString &objectName, bool separator = false);

private:
    QAction *m_sentinel;
    QPoint m_startPosition;
    bool m_interactive;
};

#endif // QDESIGNER_TOOLBAR_H
