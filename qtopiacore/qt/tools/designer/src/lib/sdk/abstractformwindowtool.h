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

#ifndef ABSTRACTFORMWINDOWTOOL_H
#define ABSTRACTFORMWINDOWTOOL_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>

QT_BEGIN_HEADER

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QWidget;
class QAction;
class DomUI;

class QDESIGNER_SDK_EXPORT QDesignerFormWindowToolInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerFormWindowToolInterface(QObject *parent = 0);
    virtual ~QDesignerFormWindowToolInterface();

    virtual QDesignerFormEditorInterface *core() const = 0;
    virtual QDesignerFormWindowInterface *formWindow() const = 0;
    virtual QWidget *editor() const = 0;

    virtual QAction *action() const = 0;

    virtual void activated() = 0;
    virtual void deactivated() = 0;

    virtual void saveToDom(DomUI*, QWidget*) {}
    virtual void loadFromDom(DomUI*, QWidget*) {}

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) = 0;
};

QT_END_HEADER

#endif // ABSTRACTFORMWINDOWTOOL_H
