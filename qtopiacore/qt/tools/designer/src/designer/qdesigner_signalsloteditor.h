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

#ifndef QDESIGNER_SIGNALSLOTEDITOR_H
#define QDESIGNER_SIGNALSLOTEDITOR_H

#include "qdesigner_toolwindow.h"

class QDesignerWorkbench;

class QDesignerSignalSlotEditor: public QDesignerToolWindow
{
    Q_OBJECT
public:
    QDesignerSignalSlotEditor(QDesignerWorkbench *workbench);
    virtual ~QDesignerSignalSlotEditor();

    virtual QRect geometryHint() const;
};

#endif // QDESIGNER_SIGNALSLOTEDITOR_H
