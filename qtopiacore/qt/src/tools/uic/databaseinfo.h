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

#ifndef DATABASEINFO_H
#define DATABASEINFO_H

#include "treewalker.h"
#include <QStringList>
#include <QMap>

class Driver;

class DatabaseInfo : public TreeWalker
{
public:
    DatabaseInfo(Driver *driver);

    void acceptUI(DomUI *node);
    void acceptWidget(DomWidget *node);

    inline QStringList connections() const
    { return m_connections; }

    inline QStringList cursors(const QString &connection) const
    { return m_cursors.value(connection); }

    inline QStringList fields(const QString &connection) const
    { return m_fields.value(connection); }

private:
    Driver *driver;
    QStringList m_connections;
    QMap<QString, QStringList> m_cursors;
    QMap<QString, QStringList> m_fields;
};

#endif // DATABASEINFO_H
