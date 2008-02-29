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

#include "qmousedriverfactory_qws.h"

#include "qapplication.h"
#include "qmousepc_qws.h"
#include "qmousebus_qws.h"
#include "qmousevr41xx_qws.h"
#include "qmouseyopy_qws.h"
#include "qmouselinuxtp_qws.h"
#include "qmousevfb_qws.h"
#include "qmousetslib_qws.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qmousedriverplugin_qws.h"
#include "qdebug.h"

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWSMouseHandlerFactoryInterface_iid, QCoreApplication::libraryPaths(),
     QLatin1String("/mousedrivers")))

#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

/*!
    \class QMouseDriverFactory
    \ingroup qws

    \brief The QMouseDriverFactory class creates mouse drivers in
    Qtopia Core.

    Note that this class is only available in \l {Qtopia Core}.

    QMouseDriverFactory is used to detect and instantiate the
    available mouse drivers, allowing \l {Qtopia Core} to load the
    preferred driver into the server application at runtime. The
    create() function returns a QWSMouseHandler object representing
    the mouse driver identified by a given key. The valid keys
    (i.e. the supported drivers) can be retrieved using the keys()
    function.

    \l {Qtopia Core} provides several built-in mouse drivers. In
    addition, custom mouse drivers can be added using Qt's plugin
    mechanism, i.e. by subclassing the QWSMouseHandler class and
    creating a mouse driver plugin (QMouseDriverPlugin). See the \l
    {Qtopia Core Pointer Handling}{pointer handling} documentation for
    details.

    \sa QWSMouseHandler, QMouseDriverPlugin
*/

/*!
    Creates the mouse driver specified by the given \a key, using the
    display specified by the given \a device.

    Note that the keys are case-insensitive.

    \sa keys()
*/
QWSMouseHandler *QMouseDriverFactory::create(const QString& key, const QString &device)
{
    QString driver = key.toLower();
#ifndef QT_NO_QWS_MOUSE_LINUXTP
    if (driver == QLatin1String("linuxtp") || driver.isEmpty())
        return new QWSLinuxTPMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_YOPY
    if (driver == QLatin1String("yopy") || driver.isEmpty())
        return new QWSYopyMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_VR41XX
    if (driver == QLatin1String("vr41xx") || driver.isEmpty())
        return new QWSVr41xxMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_PC
    if (driver == QLatin1String("auto")
        || driver == QLatin1String("intellimouse")
        || driver == QLatin1String("microsoft")
        || driver == QLatin1String("mousesystems")
        || driver == QLatin1String("mouseman")
        || driver.isEmpty()) {
        return new QWSPcMouseHandler(key, device);
    }
#endif
#ifndef QT_NO_QWS_MOUSE_BUS
    if (driver == QLatin1String("bus"))
        return new QWSBusMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_TSLIB
    if (driver == QLatin1String("tslib") || driver.isEmpty())
        return new QWSTslibMouseHandler(key, device);
#endif
#ifndef QT_NO_QWS_MOUSE_QVFB
    if (driver == QLatin1String("qvfbmouse") || driver == QLatin1String("qvfb"))
        return new QVFbMouseHandler(key, device);
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    if (QWSMouseHandlerFactoryInterface *factory = qobject_cast<QWSMouseHandlerFactoryInterface*>(loader()->instance(driver)))
        return factory->create(driver);
#endif
#endif
    return 0;
}

/*!
    Returns the list of valid keys, i.e. the available mouse drivers.

    \sa create()
*/
QStringList QMouseDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_MOUSE_LINUXTP
    list << QLatin1String("LinuxTP");
#endif
#ifndef QT_NO_QWS_MOUSE_YOPY
    list << QLatin1String("Yopy");
#endif
#ifndef QT_NO_QWS_MOUSE_VR41XX
    list << QLatin1String("VR41xx");
#endif
#ifndef QT_NO_QWS_MOUSE_PC
    list << QLatin1String("Auto")
         << QLatin1String("IntelliMouse")
         << QLatin1String("Microsoft")
         << QLatin1String("MouseSystems")
         << QLatin1String("MouseMan");
#endif
#ifndef QT_NO_QWS_MOUSE_BUS
    list << QLatin1String("Bus");
#endif
#ifndef QT_NO_QWS_MOUSE_TSLIB
    list << QLatin1String("Tslib");
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    QStringList plugins = loader()->keys();
    for (int i = 0; i < plugins.size(); ++i) {
        if (!list.contains(plugins.at(i)))
            list += plugins.at(i);
    }
#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

    return list;
}
