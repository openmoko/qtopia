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

#ifndef QSCREENDRIVERPLUGIN_QWS_H
#define QSCREENDRIVERPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_LIBRARY

class QScreen;

struct Q_GUI_EXPORT QScreenDriverFactoryInterface : public QFactoryInterface
{
    virtual QScreen* create(const QString& driver, int displayId) = 0;
};

#define QScreenDriverFactoryInterface_iid "com.trolltech.Qt.QScreenDriverFactoryInterface"
Q_DECLARE_INTERFACE(QScreenDriverFactoryInterface, QScreenDriverFactoryInterface_iid)

class Q_GUI_EXPORT QScreenDriverPlugin : public QObject, public QScreenDriverFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QScreenDriverFactoryInterface:QFactoryInterface)
public:
    explicit QScreenDriverPlugin(QObject *parent = 0);
    ~QScreenDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QScreen *create(const QString& driver, int displayId) = 0;
};

#endif // QT_NO_LIBRARY

QT_END_HEADER

#endif // QSCREENDRIVERPLUGIN_QWS_H
