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
