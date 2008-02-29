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

#ifndef BRUSHMANAGERPROXY_H
#define BRUSHMANAGERPROXY_H

#include <QtCore/QObject>

namespace qdesigner_internal {

class QtBrushManager;
class BrushManagerProxyPrivate;

class BrushManagerProxy : public QObject
{
    Q_OBJECT
public:
    BrushManagerProxy(QDesignerFormEditorInterface *core, QObject *parent = 0);
    ~BrushManagerProxy();

    void setBrushManager(QtBrushManager *manager);

private:
    BrushManagerProxyPrivate *d_ptr;
    Q_DECLARE_PRIVATE(BrushManagerProxy)
    Q_DISABLE_COPY(BrushManagerProxy)
    Q_PRIVATE_SLOT(d_func(), void brushAdded(const QString &, const QBrush &))
    Q_PRIVATE_SLOT(d_func(), void brushRemoved(const QString &name))
};

}  // namespace qdesigner_internal

#endif
