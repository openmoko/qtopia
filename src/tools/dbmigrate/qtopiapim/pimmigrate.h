/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __PIM_MIGRATE_H__
#define __PIM_MIGRATE_H__

#include <QDateTime>
#include <QString>

class QSqlDatabase;
class QDBMigrationEngine;
class QStringList;

class PimMigrate // : public DBMigrationEngine
{
public:
    PimMigrate(QDBMigrationEngine *engine);
    ~PimMigrate();

    bool migrate();
private:
    bool migrate(const QSqlDatabase &db, const QString &table, int version);
    const QStringList &tables() const;
    bool createContactEvents(const QSqlDatabase &db);
    bool createTodoEvents(const QSqlDatabase &db);

    QString queryText(const QString& type, const QString &table);
    QDateTime syncTime;
    QDBMigrationEngine *mi;
};

#endif//__PIM_MIGRATE_H__
