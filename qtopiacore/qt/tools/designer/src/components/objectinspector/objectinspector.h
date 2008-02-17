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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef OBJECTINSPECTOR_H
#define OBJECTINSPECTOR_H

#include "objectinspector_global.h"
#include "qdesigner_objectinspector_p.h"

#include <QtCore/QPointer>
#include <QtCore/QList>
#include <QtCore/QSet>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QTreeWidgetItem;
class QItemSelection;

namespace qdesigner_internal {
class FormWindowBase;
class TreeWidget;

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public QDesignerObjectInspector
{
    Q_OBJECT
public:
    ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    virtual QDesignerFormEditorInterface *core() const;

    virtual void getSelection(Selection &s) const;
    virtual bool selectObject(QObject *o);
    virtual void clearSelection();

    void setFormWindow(QDesignerFormWindowInterface *formWindow);

public slots:
    virtual void mainContainerChanged();

private slots:
    void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void slotPopupContextMenu(const QPoint &pos);
    void slotHeaderDoubleClicked(int column);

protected:
    virtual void dragEnterEvent (QDragEnterEvent * event);
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dragLeaveEvent(QDragLeaveEvent * event);
    virtual void dropEvent (QDropEvent * event);

private:
    static bool sortEntry(const QObject *a, const QObject *b);
    void showContainersCurrentPage(QWidget *widget);

private:
    void restoreDropHighlighting();
    QWidget *managedWidgetAt(const QPoint &global_mouse_pos);
    void handleDragEnterMoveEvent(QDragMoveEvent * event, bool isDragEnter);

    typedef QSet<const QObject *> PreviousSelection;
    PreviousSelection previousSelection(QDesignerFormWindowInterface *fw, bool formWindowChanged) const;

    typedef QList<QTreeWidgetItem *> ItemList;
    static void findRecursion(QTreeWidgetItem *item, QObject *o, ItemList &matchList);

    ItemList findItemsOfObject(QObject *o) const;

    QDesignerFormEditorInterface *m_core;
    TreeWidget *m_treeWidget;
    QPointer<FormWindowBase> m_formWindow;
    QPointer<QWidget> m_formFakeDropTarget;
};

}  // namespace qdesigner_internal

#endif // OBJECTINSPECTOR_H
