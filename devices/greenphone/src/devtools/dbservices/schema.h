/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef DBSERVICES_SCHEMA_H
#define DBSERVICES_SCHEMA_H

#include <QHash>
#include <QString>
#include <QSqlDatabase>

struct FieldDef
{
    bool notNull;
    bool primaryKey;
    QString name;
    QString ctype;
    QString constraints;
    QString foreignKey;  // format is "table:column-name"
};

typedef QHash<QString,FieldDef> ColumnList;

struct TableDef
{
    QString name;
    ColumnList cols;
    QString createStatement;
};

class DbSchema
{
public:
    DbSchema( QSqlDatabase &db );
    ~DbSchema();
    QString migrate( const QString &, const DbSchema * ) const;
    QString dbName() const;
    TableDef *tableDef( const QString & ) const;
private:
    QString getTypeFromCreate( const QString &, const QString & ) const;
    QString getDefaultFromType( const QString & ) const;
    QString addMissingMandatoryColumns( const TableDef * ) const;
    ColumnList getColumnListFromCreate( const QString & ) const;
    void addConstraint( ColumnList &, const QString & ) const;
    FieldDef parseField( const QString &, int & ) const;
    QString parseName( const QString &, int & ) const;
    QString parseCtype( const QString &, int & ) const;
    QString parseConstraints( const QString &, int & ) const;

    QHash<QString,TableDef*> tables;

    QSqlDatabase &db;
    bool isError;
};


inline QString DbSchema::dbName() const
{
    return db.databaseName();
}

inline TableDef *DbSchema::tableDef( const QString &t ) const
{
    return tables.contains( t ) ? tables[t] : 0;
}

#endif
