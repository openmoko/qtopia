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

#ifndef OBJECTINSPECTOR_H
#define OBJECTINSPECTOR_H

#include "objectinspector_global.h"
#include <QtDesigner/QtDesigner>
#include <QtCore/QPointer>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TreeWidget;
class ObjectItem;

class QT_OBJECTINSPECTOR_EXPORT ObjectInspector: public QDesignerObjectInspectorInterface
{
    Q_OBJECT
public:
    ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    virtual ~ObjectInspector();

    virtual QDesignerFormEditorInterface *core() const;

    void setFormWindow(QDesignerFormWindowInterface *formWindow);

private slots:
    void slotSelectionChanged();
    void slotPopupContextMenu(const QPoint &pos);

protected:
    virtual void showEvent(QShowEvent *enent);

private:
    static bool sortEntry(const QObject *a, const QObject *b);

private:
    QDesignerFormEditorInterface *m_core;
    TreeWidget *m_treeWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    ObjectItem *m_root;
    QPointer<QObject> m_selected;
    bool m_ignoreNextUpdate;
};

}  // namespace qdesigner_internal

#endif // OBJECTINSPECTOR_H
