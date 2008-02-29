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

#ifndef TASKMENU_COMPONENT_H
#define TASKMENU_COMPONENT_H

#include "taskmenu_global.h"
#include <QtDesigner/taskmenu.h>

#include <QtCore/QObject>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class QT_TASKMENU_EXPORT TaskMenuComponent: public QObject
{
    Q_OBJECT
public:
    TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~TaskMenuComponent();

    QDesignerFormEditorInterface *core() const;

private:
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

#endif // TASKMENU_COMPONENT_H
