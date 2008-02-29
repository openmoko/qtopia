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

#ifndef QDESIGNER_OBJECTINSPECTOR_H
#define QDESIGNER_OBJECTINSPECTOR_H

#include "qdesigner_toolwindow.h"

class QDesignerWorkbench;

class QDesignerObjectInspector: public QDesignerToolWindow
{
    Q_OBJECT
public:
    QDesignerObjectInspector(QDesignerWorkbench *workbench);
    virtual ~QDesignerObjectInspector();

    virtual QRect geometryHint() const;
};

#endif // QDESIGNER_OBJECTINSPECTOR_H
