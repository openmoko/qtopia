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

#ifndef ABSTRACTFORMEDITORPLUGIN_H
#define ABSTRACTFORMEDITORPLUGIN_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>

QT_BEGIN_HEADER

class QDesignerFormEditorInterface;
class QAction;

class QDESIGNER_SDK_EXPORT QDesignerFormEditorPluginInterface
{
public:
    virtual ~QDesignerFormEditorPluginInterface() {}

    virtual bool isInitialized() const = 0;
    virtual void initialize(QDesignerFormEditorInterface *core) = 0;
    virtual QAction *action() const = 0;

    virtual QDesignerFormEditorInterface *core() const = 0;
};
Q_DECLARE_INTERFACE(QDesignerFormEditorPluginInterface, "com.trolltech.Qt.Designer.QDesignerFormEditorPluginInterface")

QT_END_HEADER

#endif // ABSTRACTFORMEDITORPLUGIN_H
