/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
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
#include "../../../sql/drivers/oci/qsql_oci.h"

class QOCIDriverPlugin : public QSqlDriverPlugin
{
public:
    QOCIDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QOCIDriverPlugin::QOCIDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QOCIDriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QOCI") || name == QLatin1String("QOCI8")) {
        QOCIDriver* driver = new QOCIDriver();
        return driver;
    }
    return 0;
}

QStringList QOCIDriverPlugin::keys() const
{
    QStringList l;
    l.append(QLatin1String("QOCI8"));
    l.append(QLatin1String("QOCI"));
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QOCIDriverPlugin)
Q_EXPORT_PLUGIN2(qsqloci, QOCIDriverPlugin)
