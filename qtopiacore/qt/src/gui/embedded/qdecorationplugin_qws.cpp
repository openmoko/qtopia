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

#include "qdecorationplugin_qws.h"
#include "qdecoration_qws.h"

/*!
    \class QDecorationPlugin
    \ingroup qws
    \ingroup plugins

    \brief The QDecorationPlugin class is an abstract base class for
    window decoration plugins.

    Note that this class is only available in \l {Qtopia Core}.

    \l {Qtopia Core} provides three ready-made decoration styles: \c
    Default, \c Styled and \c Windows. Custom decorations can be
    implemented by subclassing the QDecoration class and creating a
    decoration plugin.

    A decoration plugin can be created by subclassing
    QDecorationPlugin and implementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, \l {Qtopia Core}'s implementation of the
    QDecorationFactory class will automatically detect the plugin and
    load the driver into the application at runtime. See \l{How to
    Create Qt Plugins} for details.

    To actually apply a decoration, use the
    QApplication::qwsSetDecoration() function.

    \sa QDecoration, QDecorationFactory
*/

/*!
    \fn QStringList QDecorationPlugin::keys() const

    Returns the list of valid keys, i.e., the decorations supported by
    this plugin.

    \sa create()
*/

/*!
    \fn QDecoration *QDecorationPlugin::create(const QString &key)

    Creates a decoration matching the given \a key. Note that keys are
    case-insensitive.

    \sa keys()
*/

/*!
    Constructs a decoration plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QDecorationPlugin::QDecorationPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the decoration plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QDecorationPlugin::~QDecorationPlugin()
{
}
