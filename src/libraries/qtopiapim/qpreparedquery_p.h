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

#ifndef PREPARED_QUERY_PRIVATE_H
#define PREPARED_QUERY_PRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QVariant>
#include <QSqlError>
#include <QSqlQuery>

#ifndef QT_44_API_QSQLQUERY_FINISH
#define SQLITE_DIRECT
#endif

#ifdef SQLITE_DIRECT
struct sqlite3_stmt;
struct sqlite3;
#endif

class QPreparedSqlQuery
{
public:
    QPreparedSqlQuery();
    QPreparedSqlQuery(QSqlDatabase);
    QPreparedSqlQuery(const QString &statement);
    ~QPreparedSqlQuery();

    bool isValid() const;

    bool prepare();
    bool prepare(const QString &);
    void reset();
    void clear();

    void bindValue( const QString & placeholder, const QVariant & val, QSql::ParamType paramType = QSql::In );
    void bindValue( int pos, const QVariant & val, QSql::ParamType paramType = QSql::In );

    QMap<QString, QVariant> boundValues() const;

    bool exec();
    bool isNull( int field ) const;

    QSqlError lastError() const;
    QString lastQuery() const;
    bool next();
    //bool seek( int index, bool relative = false );
    QVariant value( int index ) const;

private:
    void buildQuery();
    QString mText;
#ifdef SQLITE_DIRECT
    sqlite3_stmt *mHandle;
    sqlite3 *mDBHandle;
    QSqlError mError;
    bool skip_step;
    int step_res;
#else
    QSqlQuery *mQuery;
#endif
};

#endif
