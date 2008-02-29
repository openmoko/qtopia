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

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../../src/sql/drivers/sqlite2/qsql_sqlite2.h"

class QSQLite2DriverPlugin : public QSqlDriverPlugin
{
public:
    QSQLite2DriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QSQLite2DriverPlugin::QSQLite2DriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QSQLite2DriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QSQLITE2")) {
        QSQLite2Driver* driver = new QSQLite2Driver();
        return driver;
    }
    return 0;
}

QStringList QSQLite2DriverPlugin::keys() const
{
    QStringList l;
    l  << QLatin1String("QSQLITE2");
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QSQLite2DriverPlugin)
Q_EXPORT_PLUGIN2(qsqlite2, QSQLite2DriverPlugin)
