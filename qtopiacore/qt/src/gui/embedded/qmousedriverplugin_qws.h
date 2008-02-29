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

#ifndef QMOUSEDRIVERPLUGIN_QWS_H
#define QMOUSEDRIVERPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_LIBRARY

class QWSMouseHandler;

struct Q_GUI_EXPORT QWSMouseHandlerFactoryInterface : public QFactoryInterface
{
    virtual QWSMouseHandler* create(const QString& name) = 0;
};

#define QWSMouseHandlerFactoryInterface_iid "com.trolltech.Qt.QWSMouseHandlerFactoryInterface"
Q_DECLARE_INTERFACE(QWSMouseHandlerFactoryInterface, QWSMouseHandlerFactoryInterface_iid)

class Q_GUI_EXPORT QMouseDriverPlugin : public QObject, public QWSMouseHandlerFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QWSMouseHandlerFactoryInterface:QFactoryInterface)
public:
    explicit QMouseDriverPlugin(QObject *parent = 0);
    ~QMouseDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QWSMouseHandler* create(const QString& driver, const QString &device) = 0;
};

#endif // QT_NO_LIBRARY

QT_END_HEADER

#endif // QMOUSEDRIVERPLUGIN_QWS_H
