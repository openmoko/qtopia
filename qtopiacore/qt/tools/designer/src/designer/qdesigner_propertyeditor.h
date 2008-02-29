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

#ifndef QDESIGNER_PROPERTYEDITOR_H
#define QDESIGNER_PROPERTYEDITOR_H

#include "qdesigner_toolwindow.h"

class QDesignerWorkbench;

class QDesignerPropertyEditor: public QDesignerToolWindow
{
    Q_OBJECT
public:
    QDesignerPropertyEditor(QDesignerWorkbench *workbench);
    virtual ~QDesignerPropertyEditor();

    virtual QRect geometryHint() const;

protected:
    virtual void showEvent(QShowEvent *event);
};

#endif // QDESIGNER_PROPERTYEDITOR_H
