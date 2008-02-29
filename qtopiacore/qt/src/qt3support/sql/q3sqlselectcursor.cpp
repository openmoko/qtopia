/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#include "q3sqlselectcursor.h"
#include "qsqldriver.h"
#include "q3sqlrecordinfo.h"

#ifndef QT_NO_SQL

class Q3SqlSelectCursorPrivate
{
public:
    Q3SqlSelectCursorPrivate() : populated(false) {}
    QString query;
    bool populated : 1;
};

/*!
    \class Q3SqlSelectCursor qsqlselectcursor.h
    \brief The Q3SqlSelectCursor class provides browsing of general SQL SELECT statements.

    \compat

    Q3SqlSelectCursor is a convenience class that makes it possible to
    display result sets from general SQL \c SELECT statements in
    data-aware Qt widgets. Q3SqlSelectCursor is read-only and does not
    support \c INSERT, \c UPDATE or \c DELETE operations.

    Pass the query in at construction time, or use the
    Q3SqlSelectCursor::exec() function.

    Example:
    \code
    ...
    Q3SqlSelectCursor* cur = new Q3SqlSelectCursor("SELECT id, firstname, lastname FROM author");
    Q3DataTable* table = new Q3DataTable(this);
    table->setSqlCursor(cur, true, true);
    table->refresh();
    ...
    cur->exec("SELECT * FROM books");
    table->refresh();
    ...
    \endcode
*/

/*!
    Constructs a read only cursor on database \a db using the query \a query.
 */
Q3SqlSelectCursor::Q3SqlSelectCursor(const QString& query, QSqlDatabase db)
    : Q3SqlCursor(QString(), false, db)
{
    d = new Q3SqlSelectCursorPrivate;
    d->query = query;
    Q3SqlCursor::setMode(ReadOnly);
    if (!query.isEmpty())
        exec(query);
}

/*! Constructs a copy of \a other */
Q3SqlSelectCursor::Q3SqlSelectCursor(const Q3SqlSelectCursor& other)
    : Q3SqlCursor(other)
{
    d = new Q3SqlSelectCursorPrivate;
    d->query = other.d->query;
    d->populated = other.d->populated;
}

/*! Destroys the object and frees any allocated resources */
Q3SqlSelectCursor::~Q3SqlSelectCursor()
{
    delete d;
}

/*! \internal */
bool Q3SqlSelectCursor::exec(const QString& query)
{
    d->query = query;
    bool ret = Q3SqlCursor::exec(query);
    if (ret) {
        Q3SqlCursor::clear();
        populateCursor();
    }
    return ret;
}

/*! \fn bool Q3SqlSelectCursor::select()
    \internal
*/

/*! \internal */
bool Q3SqlSelectCursor::select(const QString&, const QSqlIndex&)
{
    bool ret = Q3SqlCursor::exec(d->query);
    if (ret && !d->populated)
        populateCursor();
    return ret;
}

/*! \internal */
void Q3SqlSelectCursor::populateCursor()
{
    Q3SqlRecordInfo inf = Q3SqlRecordInfo(record());
    for (Q3SqlRecordInfo::const_iterator it = inf.begin(); it != inf.end(); ++it)
        Q3SqlCursor::append(*it);
    d->populated = true;
}

/*! \fn QSqlIndex Q3SqlSelectCursor::primaryIndex(bool) const
    \internal
*/

/*! \fn QSqlIndex Q3SqlSelectCursor::index(const QStringList&) const
    \internal
*/

/*! \fn QSqlIndex Q3SqlSelectCursor::index(const QString&) const
    \internal
*/

/*! \fn QSqlIndex Q3SqlSelectCursor::index(const char*) const
    \internal
*/

/*! \fn void Q3SqlSelectCursor::setPrimaryIndex(const QSqlIndex&)
    \internal
*/

/*! \fn void Q3SqlSelectCursor::append(const Q3SqlFieldInfo&)
    \internal
*/

/*! \fn void Q3SqlSelectCursor::insert(int, const Q3SqlFieldInfo&)
    \internal
*/

/*! \fn void Q3SqlSelectCursor::remove(int)
    \internal
*/

/*! \fn void Q3SqlSelectCursor::clear()
    \internal
*/

/*! \fn void Q3SqlSelectCursor::setGenerated(const QString&, bool)
    \internal
*/

/*! \fn void Q3SqlSelectCursor::setGenerated(int, bool)
    \internal
*/

/*! \fn QSqlRecord* Q3SqlSelectCursor::editBuffer(bool)
    \internal
*/

/*! \fn QSqlRecord* Q3SqlSelectCursor::primeInsert()
    \internal
*/

/*! \fn QSqlRecord* Q3SqlSelectCursor::primeUpdate()
    \internal
*/

/*! \fn QSqlRecord* Q3SqlSelectCursor::primeDelete()
    \internal
*/

/*! \fn int Q3SqlSelectCursor::insert(bool)
    \internal
*/

/*! \fn int Q3SqlSelectCursor::update(bool)
    \internal
*/

/*! \fn int Q3SqlSelectCursor::del(bool)
    \internal
*/

/*! \fn void Q3SqlSelectCursor::setMode(int)
    \internal
*/

/*! \fn void Q3SqlSelectCursor::setSort(const QSqlIndex&)
    \internal
*/

/*! \fn QSqlIndex Q3SqlSelectCursor::sort() const
    \internal
*/

/*! \fn void Q3SqlSelectCursor::setFilter(const QString&)
    \internal
*/

/*! \fn QString Q3SqlSelectCursor::filter() const
    \internal
*/

/*! \fn void Q3SqlSelectCursor::setName(const QString&, bool)
    \internal
*/

/*! \fn QString Q3SqlSelectCursor::name() const
    \internal
*/

/*! \fn QString Q3SqlSelectCursor::toString(const QString&, const QString&) const
    \internal
*/
#endif // QT_NO_SQL
