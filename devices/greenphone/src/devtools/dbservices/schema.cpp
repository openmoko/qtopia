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

#include "schema.h"

#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>

#include <qtopialog.h>

DbSchema::DbSchema( QSqlDatabase &database )
    : db( database )
    , isError( false )
{
    QSqlQuery xsql( db );
    if ( xsql.exec( "SELECT name,sql FROM sqlite_master WHERE type='table'" ) &&
            xsql.first() )
    {
        do // get all fields and tables
        {
            TableDef *tbl = new TableDef;
            tbl->name = xsql.value(0).toString().simplified();
            tbl->createStatement = xsql.value(1).toString().simplified();
            tbl->cols = getColumnListFromCreate( tbl->createStatement );
            tables[tbl->name] = tbl;
        } while( xsql.next() );
    }
    else
    {
        qWarning( "Could not get schema for db %s",
                qPrintable( db.databaseName() ));
        QSqlError e = db.lastError();
        if ( e.isValid() )
            qWarning( "%s %s", qPrintable(e.databaseText()),
                    qPrintable(e.driverText()));
        isError = true;
    }
}

DbSchema::~DbSchema()
{
    QHash<QString,TableDef*>::iterator f = tables.begin();
    for ( ; f != tables.end(); f++ )
        delete f.value();
}

/*!
  Calculate an SQL query string which when executed will migrate the data
  from the \a table in this schema to the destination schema \a dst.

  The source schema, represented by this instance, is the old data being
  migrated.  The destination schema represented by \a dst is the new
  schema and is authoritative for the new schema.

  This means that tables and columns which do not exist in the \a dst will
  not be migrated, and that data will be lost.

  For this reason, if the \a table does not exist in the \a dst, then the
  query returned is empty.

  At present this method only handles SQLITE databases, for which it generates
  the "ATTACH DATABASE" sqlite specific SQL code, to attach the database
  represented by this schema.

  The string will contain SQL command terminating ";" (semi-colon)
  characters, so it should be either fed to a command line client as is,
  or broken into commands for execution.

  \b{Invariant}:  this method relies on the target database being empty
  since the referential integrity is likely not to be maintained otherwise.
*/
QString DbSchema::migrate( const QString &table, const DbSchema *dst ) const
{
    TableDef *dstTable = dst->tableDef( table );
    if ( !dstTable || !tables.contains( table ))
    {
        qLog(Sql) << "Could not migrate table" << table;
        return QString();
    }
    QString qry = QString( "ATTACH DATABASE %1 AS _source_;" ).arg( dbName() );
    qry += addMissingMandatoryColumns( dstTable );
    QString colList = "";
    QString sep = "";
    ColumnList::iterator f = dstTable->cols.begin();
    for ( ; f != dstTable->cols.end(); f++ )
    {
        colList += sep + f->name;
        sep = ", ";
    }
    qry += QString( "INSERT INTO %1 ( %2 ) SELECT %3 FROM _source_.%4;" )
        .arg( table ).arg( colList ).arg( colList ).arg( table );
    return qry;
}

/*!
  For each column which is in the \a authority table definition but is
  specified NOT NULL, and which is not in this same named table, generate
  SQL to add that column, to the \b{source} and create default values.

  Note that SQL is generted to add the columns, but the columns are not added
  to the schema.

  The column is added to the source, so that a simpler select statement can
  be done to insert the values back into the destination, without breaking
  the "NOT NULL" constraint.

  The caller must ensure that this schema contains the same-named table.
*/
QString DbSchema::addMissingMandatoryColumns( const TableDef *authority ) const
{
    QString table = authority->name;
    Q_ASSERT( !tables.contains( table ));
    ColumnList::const_iterator f = authority->cols.begin();
    QString qry;
    for ( ; f != authority->cols.end(); f++ )
    {
        if ( f->notNull && !tables[table]->cols.contains( f->name ))
        {
            if ( f->primaryKey )
            {
                qWarning( "** Cannot import primary key %s **", qPrintable( f->name ));
                continue;
            }
            QString ctype = getTypeFromCreate( f->name, authority->createStatement );
            qry += QString( "ALTER TABLE _source_.%1 ADD COLUMN %2 %3;" )
                .arg( table ).arg( f->name ).arg( ctype );
            QString def = getDefaultFromType( ctype );
            qry += QString( "UPDATE _source_.%1 SET %1 = \'%2\';" )
                .arg( table ).arg( f->name ).arg( def );
        }
    }
    return qry;
}

/*!
  Extract the string representing the type from the \a create statement
  for the column \a name.

  The caller is responsible for ensuring the \a create is simplified, ie
  that it has only single whitespace " " seperators.

  Any constraints following the type are ignored.

  Typical values returned are, eg "varchar(100)" and "INTEGER"
  as was used in the create table statement.

  Carefully read http://www.sqlite.org/datatype3.html before
  making any assumptions about the meaning of these values.
  */
QString DbSchema::getTypeFromCreate( const QString &name, const QString &create ) const
{
    int pos = create.indexOf( name ) + name.length() + 1;
    int len = create.indexOf( ' ', pos ) - pos;
    return create.mid( pos, len );
}

/*!
  Return a default value for a column with the supplied \a ctype

  \table
  \header
    \o type
    \o default
  \row
    \o INT, INTEGER
    \o 0
  \row
    \o CHAR, VARCHAR, VARCHAR(n)
    \o ''   -- the null character
  \row
    \o DATE
    \o 1970-01-01
  \row
    \o DATETIME
    \o 1970-01-01T00:00:01
  \row
    \o anything else
    \o 0
  \endtable

  Note that Qt's QDate and QDateTime datatypes are responsible
  for the formats above - SQLITE just happens to preserve them,
  but internally just treats them as strings.

  Carefully read http://www.sqlite.org/datatype3.html before
  relying on the results of inserting these values.
*/
QString DbSchema::getDefaultFromType( const QString &ctype ) const
{
    QString nullValue = "0";  // default value for INT
    if ( ctype.contains( "CHAR", Qt::CaseInsensitive ) ||
                ctype.contains( "TEXT", Qt::CaseInsensitive ))
        nullValue = ""; // default value for CHAR/VARCHAR
    else if ( ctype.contains( "DATE", Qt::CaseInsensitive ))
    {
        nullValue = "1970-01-01";
        if ( ctype.contains( "TIME", Qt::CaseInsensitive ))
            nullValue += "T00:00:01";
    }
    return nullValue;
}

/*!
  Do a recursive descent parse (a simple one) of the string \a create
  and retrieve the schema definition list of all the columns defined
  in the create statement.

  The schema is a list of fields, possibly followed by a list of constraints.

  The fields (column definitions) look like
  \code
    <name> whitespace <type> [ whitespace <constraints> ]
  \encode

  The constraints can look like almost anything, since SQLITE ignores
  \b{silently} any constraints it doesn't understand.

  \sa parseField()
*/
ColumnList DbSchema::getColumnListFromCreate( const QString &create ) const
{
    ColumnList clist;
    FieldDef def;
    int ptr = 0;
    QString constraint;
    QString s = create + ')'; // guarantee termination
    // parse up to the beginning of the fields list
    while ( create[ptr] != '(' ) { ptr++; }
    ptr++;
    // parse out all the fields
    do
    {
        if ( constraint.isEmpty() )
        {
            def = parseField( create, ptr );
            if ( def.name == "__CONSTRAINT__" ) // oops, got a constraint
                constraint = def.constraints;
        }
        else // ahhh, we're into constraints now
        {
            constraint = parseConstraints( s, ptr );
        }
        if ( constraint.isEmpty() ) // still doing fields
            clist[def.name] = def;
        else
            addConstraint( clist, constraint );
    }
    while ( create[ptr] != ')' );
    return clist;
}

/*!
  Process a \a constraint (as opposed to a column constraint).  These \b{follow} the
  list of column definitions and include a reference to a column name in
  round brackets.  See the sqlite documentation for the syntax:
  \l{http://www.sqlite.org/lang_createtable.html} {Sql Lite Create table}

  Examples are:
  \code
     PRIMARY KEY(cid)
     UNIQUE(recid, edate),
     FOREIGN KEY(recid) REFERENCES appointments(recid));
  \endcode

  In the last case, FOREIGN KEY, sqlite parses but does not recognize or
  enforce the constraint.  However it is useful for referential integrity
  maintainenance which will be implemented in a future version of this tool.

  The caller is responsible for ensuring the \a constraint is QString::simplified.

  The \a clist is searched for matching columns and the constraints for
  that field are then updated with the new constraint info.
*/
void DbSchema::addConstraint( ColumnList &clist, const QString &constraint ) const
{
    QString cKeyWord;
    if ( constraint.contains( "UNIQUE", Qt::CaseInsensitive ))
        cKeyWord = "UNIQUE";
    else if ( constraint.contains( "PRIMARY KEY", Qt::CaseInsensitive ))
        cKeyWord = "PRIMARY KEY";
    else if ( constraint.contains( "FOREIGN KEY", Qt::CaseInsensitive ))
        cKeyWord = "FOREIGN KEY";
    if ( cKeyWord.isEmpty() )
    {
        qLog(Sql) << "constraint not known by Schema parser" << constraint;
        return;
    }
    QString s = constraint;
    int ptr = 0;
    while ( s[ptr++] != '(' ) {}  // point to after the first '('
    do
    {
        QString name = parseName( s, ptr );
        if ( clist.contains( name ))
        {
            if ( cKeyWord == "FOREIGN KEY" )
            {
                // TODO - this should be parsed out into "table:column-name"
                clist[name].foreignKey = constraint;
            }
            else
            {
                clist[name].constraints += cKeyWord;
            }
        }
    }
    while ( s[ptr] != ')' );
}

/*!
  Parse a complete field in the recursion.

  Take a create statement being processed in \a s, and the current
  parse position \a ptr (a modifiable reference).

  Caller must ensure \a ptr is pointing to the character after the
  opening '(' character of the column definition

  On return \a ptr is pointing to the closing ')' or ','.

  \sa getColumnListFromCreate(), parseName(), parseCtype(), parseConstraints()
*/
FieldDef DbSchema::parseField( const QString &s, int &ptr ) const
{
    FieldDef def;
    int savePtr = ptr;
    def.name = parseName( s, ptr );
    if ( s[ptr] == '(' ) // got some sort of constraint field, not a name
    {
        def.name = "__CONSTRAINT__";
        ptr = savePtr;
        def.constraints = parseConstraints( s, ptr );
    }
    else
    {
        def.ctype = parseCtype( s, ptr );
        int brkt = def.ctype.indexOf( '(' );
        if ( brkt != -1 )
        {
            QString rootType = def.ctype.left( brkt );
            if ( rootType == "KEY" || rootType == "UNIQUE" )
            {
                // got some sort of constraint field, not a name
                def.constraints = def.name + " " + def.ctype;
                def.name = "__CONSTRAINT__";
            }
        }
    }
    if ( def.name != "__CONSTRAINT__" )
    {
        def.constraints = parseConstraints( s, ptr );
        def.notNull = def.constraints.contains(
                "NOT NULL", Qt::CaseInsensitive );
        def.primaryKey = def.constraints.contains(
                "PRIMARY KEY", Qt::CaseInsensitive );
    }
    return def;
}

/*!
  Take a create statement being processed in \a s, and the current
  parse position \a ptr (a modifiable reference).

  Parse out the name, which appears first in the field declaration.

  The name is a sequence of non-space characters, bounded by spaces.
  \code
      " appointmentDate "
  \endcode

  At completion, \a ptr is pointing to the space character after the
  name.
*/
QString DbSchema::parseName( const QString &s, int &ptr ) const
{
    QString name = "";
    while ( s[ptr].isSpace() ) { ptr++; }
    do
    {
        name += s[ptr++];
    }
    while ( !s[ptr].isSpace() && s[ptr] != '(' && s[ptr] != ')' );
    return name;
}

/*!
  Take a create statement being processed in \a s, and the current
  parse position \a ptr (a modifiable reference).

  Parse out the column type, which appears after then name, in the
  field declaration.

  The type is a sequence of non-space characters, optionally followed
  by a '(', a sequence of any characters, and a ')'

  It is followed either by a comma (terminating this column, and
  starting the next) or a space (before the constraints), or a
  bracket ')' (terminating the whole create statement).

  \code
      " INTEGER "
      " CHAR(2) "
      " VARCHAR(10, 2) "
  \endcode

  At completion, \a ptr is pointing to the space or comma character
  after the type definition.
*/
QString DbSchema::parseCtype( const QString &s, int &ptr ) const
{
    QString ctype = "";
    while ( s[ptr++].isSpace() ) {}
    do
    {
        ctype += s[ptr];
        if ( s[ptr] == '(' )
        {
            ptr++;
            while ( s[ptr] != ')' ) ctype += s[ptr++];
        }
        ptr++;
    } while ( !s[ptr].isSpace() && s[ptr] != ',' && s[ptr] != ')' );
    return ctype;
}

/*!
  Take a create statement being processed in \a s, and the current
  parse position \a ptr (a modifiable reference).

  Parse out the constraints, an optional sequence, which appears
  after the column type.

  The constraints are a sequence of characters, terminated by either
  a comma ',' (terminating this column, and starting the next) or a
  bracket ')' (terminating the whole create statement).

  \code
      " NOT NULL, "
      " UNIQUE )"
      " PRIMARY KEY  ,"
  \endcode

  At completion, \a ptr is pointing to the bracket or comma character
  at the end of the column definition
*/
QString DbSchema::parseConstraints( const QString &s, int &ptr ) const
{
    QString constraints = "";
    while ( s[ptr].isSpace() ) { ptr++; }
    while ( s[ptr] != ',' || s[ptr] != ')' )
        constraints += s[ptr++];
    return constraints.simplified();
}
