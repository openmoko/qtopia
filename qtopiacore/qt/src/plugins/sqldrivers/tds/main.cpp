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

#define Q_UUIDIMPL
#include <qsqldriverplugin.h>
#include <qstringlist.h>
#ifdef Q_OS_WIN32    // We assume that MS SQL Server is used. Set Q_USE_SYBASE to force Sybase.
// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#include <windows.h>
#endif
#include "../../../sql/drivers/tds/qsql_tds.h"


class QTDSDriverPlugin : public QSqlDriverPlugin
{
public:
    QTDSDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QTDSDriverPlugin::QTDSDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QTDSDriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QTDS") || name == QLatin1String("QTDS7")) {
        QTDSDriver* driver = new QTDSDriver();
        return driver;
    }
    return 0;
}

QStringList QTDSDriverPlugin::keys() const
{
    QStringList l;
    l.append(QLatin1String("QTDS7"));
    l.append(QLatin1String("QTDS"));
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QTDSDriverPlugin)
Q_EXPORT_PLUGIN2(qsqltds, QTDSDriverPlugin)
