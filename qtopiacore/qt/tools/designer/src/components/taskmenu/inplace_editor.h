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

#ifndef INPLACE_EDITOR_H
#define INPLACE_EDITOR_H

#include <QtGui/QLineEdit>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class InPlaceEditor: public QLineEdit
{
    Q_OBJECT
public:
    InPlaceEditor(QWidget *widget, QDesignerFormWindowInterface *fw);
    virtual ~InPlaceEditor();

    virtual bool eventFilter(QObject *object, QEvent *event);

private:
    QWidget *m_widget;
    bool m_noChildEvent;
};

}  // namespace qdesigner_internal

#endif // INPLACE_EDITOR_H
