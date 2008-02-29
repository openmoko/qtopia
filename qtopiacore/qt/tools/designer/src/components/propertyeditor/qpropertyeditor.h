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
