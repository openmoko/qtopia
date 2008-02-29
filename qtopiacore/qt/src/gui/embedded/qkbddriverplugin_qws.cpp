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

#include "qkbddriverplugin_qws.h"

#ifndef QT_NO_LIBRARY

#include "qkbd_qws.h"

/*!
    \class QKbdDriverPlugin
    \ingroup plugins
    \ingroup qws

    \brief The QKbdDriverPlugin class is an abstract base class for
    keyboard driver plugins.

    Note that this class is only available in \l {Qtopia Core}.

    \l {Qtopia Core} provides ready-made drivers for several keyboard
    protocols, see the \l {Qtopia Core Character Input}{character
    input} documentation for details. Custom keyboard drivers can be
    implemented by subclassing the QWSKeyboardHandler class and
    creating a keyboard driver plugin.

    A keyboard driver plugin can be created by subclassing
    QKbdDriverPlugin and reimplementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, \l {Qtopia Core}'s implementation of the
    QKbdDriverFactory class will automatically detect the plugin and
    load the driver into the server application at runtime. See \l
    {How to Create Qt Plugins} for details.

    \sa QKbdDriverFactory, QWSKeyboardHandler
*/

/*!
    \fn QStringList QKbdDriverPlugin::keys() const

    Implement this function to return the list of valid keys, i.e. the
    keyboard drivers supported by this plugin.

    \l {Qtopia Core} provides ready-made drivers for several keyboard
    protocols, see the \l {Qtopia Core Character Input}{character
    input} documentation for details.

    \sa create()
*/

/*!
    Constructs a keyboard driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QKbdDriverPlugin::QKbdDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the keyboard driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QKbdDriverPlugin::~QKbdDriverPlugin()
{
}

/*!
    \fn QScreen *QKbdDriverPlugin::create(const QString &key, const QString &device)

    Implement this function to create a driver matching the type
    specified by the given \a key and \a device parameters. Note that
    keys are case-insensitive.

    \sa keys()
*/

#endif // QT_NO_LIBRARY
