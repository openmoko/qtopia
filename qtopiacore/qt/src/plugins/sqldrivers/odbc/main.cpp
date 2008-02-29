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

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/odbc/qsql_odbc.h"

class QODBCDriverPlugin : public QSqlDriverPlugin
{
public:
    QODBCDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QODBCDriverPlugin::QODBCDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QODBCDriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QODBC") || name == QLatin1String("QODBC3")) {
        QODBCDriver* driver = new QODBCDriver();
        return driver;
    }
    return 0;
}

QStringList QODBCDriverPlugin::keys() const
{
    QStringList l;
    l.append(QLatin1String("QODBC3"));
    l.append(QLatin1String("QODBC"));
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QODBCDriverPlugin)
Q_EXPORT_PLUGIN2(qsqlodbc, QODBCDriverPlugin)
