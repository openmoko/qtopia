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

#ifndef ABSTRACTACTIONEDITOR_H
#define ABSTRACTACTIONEDITOR_H

#include <QtDesigner/sdk_global.h>

#include <QtGui/QWidget>

QT_BEGIN_HEADER

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QDESIGNER_SDK_EXPORT QDesignerActionEditorInterface: public QWidget
{
    Q_OBJECT
public:
    QDesignerActionEditorInterface(QWidget *parent, Qt::WindowFlags flags = 0);
    virtual ~QDesignerActionEditorInterface();

    virtual QDesignerFormEditorInterface *core() const;

    virtual void manageAction(QAction *action) = 0;
    virtual void unmanageAction(QAction *action) = 0;

public Q_SLOTS:
    virtual void setFormWindow(QDesignerFormWindowInterface *formWindow) = 0;
};

QT_END_HEADER

#endif // ABSTRACTACTIONEDITOR_H
