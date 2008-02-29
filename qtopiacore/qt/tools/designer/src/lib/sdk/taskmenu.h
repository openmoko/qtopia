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

#ifndef TASKMENU_H
#define TASKMENU_H

#include <QtDesigner/extension.h>

QT_BEGIN_HEADER

class QAction;

class QDesignerTaskMenuExtension
{
public:
    virtual ~QDesignerTaskMenuExtension() {}

    virtual QAction *preferredEditAction() const;

    virtual QList<QAction*> taskActions() const = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerTaskMenuExtension, "com.trolltech.Qt.Designer.TaskMenu")


inline QAction *QDesignerTaskMenuExtension::preferredEditAction() const
{ return 0; }

QT_END_HEADER

#endif // TASKMENU_H
