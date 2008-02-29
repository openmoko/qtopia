/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __PHONE_MIGRATE_H__
#define __PHONE_MIGRATE_H__

#include <QDateTime>
#include <QString>

class QSqlDatabase;
class QDBMigrationEngine;
class QStringList;

class PhoneMigrate // : public DBMigrationEngine
{
public:
    PhoneMigrate(QDBMigrationEngine *engine);
    ~PhoneMigrate();

    bool migrate();
private:
    bool migrate(const QSqlDatabase &db, const QString &table, int version);
    const QStringList &tables() const;

    QString copyText(const QString &table);
    QDateTime syncTime;
    QDBMigrationEngine *mi;
};

#endif//__PHONE_MIGRATE_H__
