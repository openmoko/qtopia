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
