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

#include "qsqldriverplugin.h"

/*!
    \class QSqlDriverPlugin
    \brief The QSqlDriverPlugin class provides an abstract base for custom QSqlDriver plugins.

    \ingroup plugins
    \module sql

    The SQL driver plugin is a simple plugin interface that makes it
    easy to create your own SQL driver plugins that can be loaded
    dynamically by Qt.

    Writing a SQL plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the Q_EXPORT_PLUGIN2() macro. See the SQL
    plugins that come with Qt for example implementations (in the
    \c{plugins/src/sqldrivers} subdirectory of the source
    distribution).

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QSqlDriverPlugin::keys() const

    Returns the list of drivers (keys) this plugin supports.

    These keys are usually the class names of the custom drivers that
    are implemented in the plugin.

    \sa create()
*/

/*!
    \fn QSqlDriver *QSqlDriverPlugin::create(const QString& key)

    Creates and returns a QSqlDriver object for the driver called \a
    key. The driver key is usually the class name of the required
    driver. Keys are case sensitive.

    \sa keys()
*/

/*!
    Constructs a SQL driver plugin and sets the parent to \a parent.
    This is invoked automatically by the Q_EXPORT_PLUGIN2() macro.
*/

QSqlDriverPlugin::QSqlDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the SQL driver plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QSqlDriverPlugin::~QSqlDriverPlugin()
{
}
