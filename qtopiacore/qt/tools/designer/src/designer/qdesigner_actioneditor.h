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

#ifndef QDESIGNER_ACTIONEDITOR_H
#define QDESIGNER_ACTIONEDITOR_H

#include "qdesigner_toolwindow.h"

class QDesignerWorkbench;

class QDesignerActionEditor: public QDesignerToolWindow
{
    Q_OBJECT
public:
    QDesignerActionEditor(QDesignerWorkbench *workbench);
    virtual ~QDesignerActionEditor();

    virtual QRect geometryHint() const;
};

#endif // QDESIGNER_ACTIONEDITOR_H
