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

#ifndef QDESIGNER_WIDGETBOX_H
#define QDESIGNER_WIDGETBOX_H

#include "qdesigner_toolwindow.h"

class QDesignerWorkbench;

class QDesignerWidgetBox: public QDesignerToolWindow
{
    Q_OBJECT
public:
    QDesignerWidgetBox(QDesignerWorkbench *workbench);
    virtual ~QDesignerWidgetBox();

    virtual Qt::DockWidgetArea dockWidgetAreaHint() const;
    virtual QRect geometryHint() const;
};

#endif // QDESIGNER_WIDGETBOX_H
