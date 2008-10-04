/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qkbddriverfactory_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#include "qapplication.h"
#include "qkbdtty_qws.h"
#include "qkbdusb_qws.h"
#include "qkbdum_qws.h"
#include "qkbdsl5000_qws.h"
#include "qkbdvfb_qws.h"
#include "qkbdyopy_qws.h"
#include "qkbdvr41xx_qws.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qkbddriverplugin_qws.h"

QT_BEGIN_NAMESPACE

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QWSKeyboardHandlerFactoryInterface_iid,
     QLatin1String("/kbddrivers"), Qt::CaseInsensitive))

#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

/*!
    \class QKbdDriverFactory
    \ingroup qws

    \brief The QKbdDriverFactory class creates keyboard drivers in
    Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    QKbdDriverFactory is used to detect and instantiate the available
    keyboard drivers, allowing \l{Qt for Embedded Linux} to load the preferred
    driver into the server application at runtime. The create()
    function returns a QWSKeyboardHandler object representing the
    keyboard driver identified by a given key. The valid keys
    (i.e. the supported drivers) can be retrieved using the keys()
    function.

    \l{Qt for Embedded Linux} provides several built-in keyboard drivers. In
    addition, custom keyboard drivers can be added using Qt's plugin
    mechanism, i.e. by subclassing the QWSKeyboardHandler class and
    creating a keyboard driver plugin (QKbdDriverPlugin). See the
    \l{Qt for Embedded Linux Character Input}{character input} documentation
    for details.

    \sa QWSKeyboardHandler, QKbdDriverPlugin
*/

/*!
    Creates the keyboard driver specified by the given \a key, using
    the display specified by the given \a device.

    Note that the keys are case-insensitive.

    \sa keys()
*/
QWSKeyboardHandler *QKbdDriverFactory::create(const QString& key, const QString& device)
{
    QString driver = key.toLower();
#ifndef QT_NO_QWS_KBD_SL5000
    if (driver == QLatin1String("sl5000") || driver.isEmpty())
        return new QWSSL5000KeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    if (driver == QLatin1String("yopy") || driver.isEmpty())
        return new QWSYopyKeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KBD_VR41XX
    if (driver == QLatin1String("vr41xx") || driver.isEmpty())
        return new QWSVr41xxKeyboardHandler(device);
#endif
#ifndef QT_NO_QWS_KEYBOARD
# ifndef QT_NO_QWS_KBD_TTY
    if (driver == QLatin1String("tty") || driver.isEmpty())
        return new QWSTtyKeyboardHandler(device);
# endif
# ifndef QT_NO_QWS_KBD_USB
    if (driver == QLatin1String("usb"))
        return new QWSUsbKeyboardHandler(device);
# endif
# ifndef QT_NO_QWS_KBD_UM
    if (driver == QLatin1String("um") || driver == QLatin1String("qvfbkeyboard"))
        return new QWSUmKeyboardHandler(device);
# endif
# ifndef QT_NO_QWS_KBD_QVFB
    if (driver == QLatin1String("qvfbkbd")
        || driver == QLatin1String("qvfbkeyboard")
        || driver == QLatin1String("qvfb"))
        return new QVFbKeyboardHandler(device);
# endif
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    if (QWSKeyboardHandlerFactoryInterface *factory = qobject_cast<QWSKeyboardHandlerFactoryInterface*>(loader()->instance(driver)))
        return factory->create(driver, device);
#endif
#endif
    return 0;
}

/*!
    Returns the list of valid keys, i.e. the available keyboard
    drivers.

    \sa create()
*/
QStringList QKbdDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_KBD_SL5000
    list << QLatin1String("SL5000");
#endif
#ifndef QT_NO_QWS_KBD_YOPY
    list << QLatin1String("YOPY");
#endif
#ifndef QT_NO_QWS_KBD_VR41XX
    list << QLatin1String("VR41xx");
#endif
#ifndef QT_NO_QWS_KBD_TTY
    list << QLatin1String("TTY");
#endif
#ifndef QT_NO_QWS_KBD_USB
    list << QLatin1String("USB");
#endif
#ifndef QT_NO_QWS_KBD_UM
    list << QLatin1String("UM");
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

QT_END_NAMESPACE

#endif // QT_NO_QWS_KEYBOARD
