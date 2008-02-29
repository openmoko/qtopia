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

#ifndef QPROPERTYEDITOR_H
#define QPROPERTYEDITOR_H

#include "propertyeditor_global.h"
#include "qpropertyeditor_items_p.h"

#include <QtGui/QTreeView>

namespace qdesigner_internal {

class QPropertyEditorModel;
class QPropertyEditorDelegate;

class QT_PROPERTYEDITOR_EXPORT QPropertyEditor: public QTreeView
{
    Q_OBJECT
public:
    QPropertyEditor(QWidget *parent = 0);
    ~QPropertyEditor();

    IProperty *initialInput() const;
    bool isReadOnly() const;

    inline QPropertyEditorModel *editorModel() const
    { return m_model; }

signals:
    void propertyChanged(IProperty *property);

public slots:
    void setInitialInput(IProperty *initialInput);
    void setReadOnly(bool readOnly);

protected:
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual QStyleOptionViewItem viewOptions() const;
    virtual void focusInEvent(QFocusEvent *event);

private:
    QPropertyEditorModel *m_model;
    QPropertyEditorDelegate *m_itemDelegate;
    bool contentsResized;
};

}  // namespace qdesigner_internal

#endif // QPROPERTYEDITOR_H
