/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/db2/qsql_db2.h"

class QDB2DriverPlugin : public QSqlDriverPlugin
{
public:
    QDB2DriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QDB2DriverPlugin::QDB2DriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QDB2DriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QDB2")) {
        QDB2Driver* driver = new QDB2Driver();
        return driver;
    }
    return 0;
}

QStringList QDB2DriverPlugin::keys() const
{
    QStringList l;
    l.append(QLatin1String("QDB2"));
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QDB2DriverPlugin)
Q_EXPORT_PLUGIN2(qsqldb2, QDB2DriverPlugin)
