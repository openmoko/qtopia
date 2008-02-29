/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/psql/qsql_psql.h"

class QPSQLDriverPlugin : public QSqlDriverPlugin
{
public:
    QPSQLDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QPSQLDriverPlugin::QPSQLDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QPSQLDriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QPSQL") || name == QLatin1String("QPSQL7")) {
        QPSQLDriver* driver = new QPSQLDriver();
        return driver;
    }
    return 0;
}

QStringList QPSQLDriverPlugin::keys() const
{
    QStringList l;
    l.append(QLatin1String("QPSQL7"));
    l.append(QLatin1String("QPSQL"));
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QPSQLDriverPlugin)
Q_EXPORT_PLUGIN2(qsqlpsql, QPSQLDriverPlugin)
