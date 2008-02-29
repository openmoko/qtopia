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

#ifndef QKBDDRIVERPLUGIN_QWS_H
#define QKBDDRIVERPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_LIBRARY

class QWSKeyboardHandler;

struct Q_GUI_EXPORT QWSKeyboardHandlerFactoryInterface : public QFactoryInterface
{
    virtual QWSKeyboardHandler* create(const QString& name) = 0;
};

#define QWSKeyboardHandlerFactoryInterface_iid "com.trolltech.Qt.QWSKeyboardHandlerFactoryInterface"
Q_DECLARE_INTERFACE(QWSKeyboardHandlerFactoryInterface, QWSKeyboardHandlerFactoryInterface_iid)

class Q_GUI_EXPORT QKbdDriverPlugin : public QObject, public QWSKeyboardHandlerFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QWSKeyboardHandlerFactoryInterface:QFactoryInterface)
public:
    explicit QKbdDriverPlugin(QObject *parent = 0);
    ~QKbdDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QWSKeyboardHandler* create(const QString& driver, const QString &device) = 0;
};

#endif // QT_NO_LIBRARY

QT_END_HEADER

#endif // QKBDDRIVERPLUGIN_QWS_H
