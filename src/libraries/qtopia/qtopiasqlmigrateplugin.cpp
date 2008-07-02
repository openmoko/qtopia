/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <qtopiasqlmigrateplugin_p.h>

/*!
    \class QtopiaSqlMigratePlugin
    \brief The QtopiaSqlMigratePlugin class provides an interface for a database maintenance agent.
    \internal

    To minimize ongoing overhead and simplify replacement the database migration functionality of
    QtopiaSql is implemented in a plug-in which can be dynamically loaded as required and
    subsequently unloaded again when no longer needed.
*/

/*!
    Destroys a database migration plug-in.
*/
QtopiaSqlMigratePlugin::~QtopiaSqlMigratePlugin()
{
}

/*!
    \fn QtopiaSqlMigratePlugin::migrate(QSqlDatabase *database)

    Ensures a database has the latest schema.

    This will also attempt to discover and repair any corruption in the database.  In the case that
    the database is corrupted beyond repair a backup will be created and a replacement created.

    Returns true if the schema is up to date and false if the database is unusable for any reason.

    \sa QtopiaSql::attachDB()
*/
