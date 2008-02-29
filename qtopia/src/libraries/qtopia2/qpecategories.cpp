/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qpecategories.h"
#include "qpesql.h"

class QPECategoryManagerPrivate
{
public:
    QPECategoryManagerPrivate() : hasAltDest(FALSE) {}
    bool hasAltDest;
    QString dest;
};

QPECategoryManager::QPECategoryManager( QObject *parent, const char *name )
    : QObject(parent, name)
{
    d = new QPECategoryManagerPrivate();
    QPESql::openDatabase();
    initTables();
}

#ifdef QTOPIA_DESKTOP
// doesn't make sense other than on the desktop.
QPECategoryManager::QPECategoryManager( const QCString &installLocal, QObject *parent, const char *name )
    : QObject(parent, name)
{
    d = new QPECategoryManagerPrivate();
    d->hasAltDest = TRUE;
    d->dest = installLocal;
    // FIXME: Needs to specify which database.
    QPESql::openDatabase();
    initTables();
}
#endif

void QPECategoryManager::initTables()
{
    // create tables 
    QSqlQuery q;
    // get tables, for each table not created do...

    q.exec("CREATE TABLE categoryMap (catid int, catname varchar, appid int, primary key(catid), unique (catname, appid))");
    q.exec("CREATE TABLE appMap (appid int, appname varchar, primary key(appid), unique (appname))");
    q.exec("CREATE TABLE categories (appid int, recid int, catid, unique(appid, recid, catid))");
    q.exec("CREATE INDEX appIndex on categories (appid)"); // as so many querys will use this.

    // need the following constraints..
    // all appid in appMap. (from cm & c)
    // all catid in cateogryMap

    // insert into app id 1 = global
    // insert catid 1 = Business and 2 = Personal into table.
    // mark as system?
}

QPECategoryManager::QPECategoryManager( const QPECategoryManager &copyFrom )
{
    d = new QPECategoryManagerPrivate();
    d->hasAltDest = copyFrom.d->hasAltDest;
    d->dest = copyFrom.d->dest;
}

QPECategoryManager::~QPECategoryManager()
{
    //QPESql::closeDatabase();
    delete d;
}

QPECategoryManager &QPECategoryManager::operator=( const QPECategoryManager &c )
{
    d->hasAltDest = c.d->hasAltDest;
    d->dest = c.d->dest;
    return *this;
}

void QPECategoryManager::clearAll()
{
    QSqlQuery q;
    q.exec("DELETE FROM categories");
    q.exec("DELETE FROM categoryMap WHERE catid NOT IN (1,2)");
}
void QPECategoryManager::clearGlobal()
{
    QSqlQuery q;
    q.exec("DELETE FROM categories WHERE catid in (SELECT catid from categoryMap where appid = 1)");
    // above, ?? and catid not in (1,2);
    q.exec("DELETE FROM categoryMap WHERE catid NOT IN (1,2) AND appid = 1");
}
void QPECategoryManager::clear( const QCString &appname )
{
    QSqlQuery q;
    q.exec("DELETE FROM categories WHERE catid in (SELECT catid from categoryMap where appid != 1)");
    q.exec(QString("DELETE FROM categoryMap WHERE appid in (SELECT appid from appMap where appname = '%1'")
	    .arg(appname));
}

// some categories are magic.
// CANNOT RENAME, DELETE, OR RESCOPE these categories.
QString QPECategoryManager::businessCategory() const
{
    QSqlQuery q;
    q.exec("SELECT catname FROM categoryMap WHERE catid = 1");
    if (q.next())
	return q.value(0).toString();
    return QString::null;
}

QString QPECategoryManager::personalCategory() const
{
    QSqlQuery q;
    q.exec("SELECT catname FROM categoryMap WHERE catid = 2");
    if (q.next())
	return q.value(0).toString();
    return QString::null;
}

bool QPECategoryManager::isSystemCategory( const QString &catname ) const
{
    QString query("SELECT catid FROM categoryMap WHERE catname = '%1' AND catid IN (1,2)");
    QSqlQuery q;
    q.exec(query.arg(catname));
    if (q.next())
	return TRUE;
    return FALSE;
}

// only checks global categories
bool QPECategoryManager::isGlobal( const QString &catname ) const
{
    QString query("SELECT catid FROM categoryMap WHERE catname = '%1' AND appid = 1");
    QSqlQuery q;
    q.exec(query.arg(catname));
    if (q.next())
	return TRUE;
    return FALSE;
}
// checks all categories
bool QPECategoryManager::contains( const QString &catname ) const
{
    QString query("SELECT catid FROM categoryMap WHERE catname = '%1'");
    QSqlQuery q;
    q.exec(query.arg(catname));
    if (q.next())
	return TRUE;
    return FALSE;
}
// checks only one app or group
bool QPECategoryManager::contains( const QString &appname, const QString &catname) const
{
    QString query("SELECT catid FROM categoryMap WHERE catname = '%1' AND appid in (SELECT appid FROM appMap WHERE appname = '%2')");
    QSqlQuery q;
    q.exec(query.arg(catname).arg(appname));
    if (q.next())
	return TRUE;
    return FALSE;
}

QStringList QPECategoryManager::categories( const QCString &appname ) const
{
    QString query("SELECT catname FROM categoryMap WHERE appid in (SELECT appid FROM appMap WHERE appname = '%1')");
    QSqlQuery q;
    q.exec(query.arg(appname));
    QStringList l;
    while (q.next()) {
	l.append(q.value(0).toString());
    }
    return l;
}

// list of only global categories
QStringList QPECategoryManager::globalCategories() const
{
    QString query("SELECT catname FROM categoryMap WHERE appid = 1");
    QSqlQuery q;
    q.exec(query);
    QStringList l;
    while (q.next()) {
	l.append(q.value(0).toString());
    }
    return l;
}

// sort of a reverse.  Which apps have this category.
QValueList<QCString> QPECategoryManager::groups( const QString &category ) const
{
    // select appname from categoryMap where catname = category;
    QString query("SELECT appname FROM appMap WHERE appid IN (SELECT appid FROM categoryMap WHERE catname = '%1'");
    QSqlQuery q;
    q.exec(query.arg(category));
    QValueList<QCString> l;
    while (q.next()) {
	l.append(q.value(0).toCString());
    }
    return l;

    return QValueList<QCString>();
}

// implies adding the group appname
bool QPECategoryManager::addCategory( const QCString &appname, const QString &catname )
{
    // Yes, the is legal sql :)
    QString addapp("INSERT INTO appMap (appname) VALUES ('%1')");
    QString addcat("INSERT INTO categoryMap (catname, appid) SELECT '%1', appid FROM appMap WHERE appname = '%2'");

    // don't care if fail, it means only care about result.
    QSqlQuery q;
    q.exec(addapp.arg(appname));

    if (q.exec(addcat.arg(catname).arg(appname)))
	return TRUE;
    return FALSE;
}

bool QPECategoryManager::addGlobalCategory( const QString &catname )
{
    QString addcat("INSERT INTO categoryMap (catname, appid) VALUES ('%1', 1)");

    QSqlQuery q;
    if (q.exec(addcat.arg(catname)))
	return TRUE;
    return FALSE;
}

// fails if not scoped to app, or if appName not in db. (or if system cat)
bool QPECategoryManager::removeCategory( const QCString &appName, const QString &catName)
{
    if (isSystemCategory(catName))
	return FALSE;
    QString delrec("DELETE FROM categories WHERE catid IN (SELECT catid FROM categoryMap WHERE catname = '%1'"
	    " AND appid IN (SELECT appid FROM appMap WHERE appname = '%2'))");
    QString delcat("DELETE FROM categoryMap WHERE catname = '%1' AND appid IN (SELECT appid FROM appMap WHERE appname = '%2')");
    QSqlQuery q;
    q.exec(delrec.arg(catName).arg(appName));
    if (q.exec(delcat.arg(catName).arg(appName)))
	return TRUE;
    return FALSE;
}
bool QPECategoryManager::removeGlobalCategory( const QString &catName )
{
    if (isSystemCategory(catName))
	return FALSE;
    QString delrec("DELETE FROM categories WHERE catid IN (SELECT catid FROM categoryMap WHERE catname = '%1'"
	    " AND appid = 1)");
    QString delcat("DELETE FROM categoryMap WHERE catname = '%1' AND appid = 1");
    QSqlQuery q;
    q.exec(delrec.arg(catName));
    if (q.exec(delcat.arg(catName)))
	return TRUE;
    return FALSE;
}

bool QPECategoryManager::renameCategory( const QString &appname,
	const QString &oldname,
	const QString &newname )
{
    if (isSystemCategory(oldname))
	return FALSE;

    QString query("UPDATE categoryMap SET catname = '%1' WHERE catname = '%2' AND appid in (SELECT appid FROM appMap WHERE appname = '%3')");
    QSqlQuery q;

    if (q.exec(query.arg(newname).arg(oldname).arg(appname)))
	return TRUE;
    return FALSE;
}

bool QPECategoryManager::renameGlobalCategory( const QString &oldname,
	const QString &newname )
{
    if (isSystemCategory(oldname))
	return FALSE;

    QString query("UPDATE categoryMap SET catname = '%1' WHERE catname = '%2' AND appid = 1");
    QSqlQuery q;

    if (q.exec(query.arg(newname).arg(oldname)))
	return TRUE;
    return FALSE;
}

void QPECategoryManager::makeGlobal( const QString &appname, const QString &catname)
{
    // HARD to achieve given dependencies.  will transactions save us?
}

void QPECategoryManager::makeLocal( const QString &catname )
{
    // HARD to achieve given dependencies, will transactions save us?
}


// the reset are convienence, flesh out later.
QPECategoryGroup::QPECategoryGroup(const QCString &)
{
} // app related

QPECategoryGroup::QPECategoryGroup()
{
} // global only
QPECategoryGroup::QPECategoryGroup( const QPECategoryGroup &c )
{
}

void QPECategoryGroup::clear()
{
}

bool QPECategoryGroup::add( const QString &label )
{
    return FALSE;
}
bool QPECategoryGroup::addGlobal( const QString &label )
{
    return FALSE;
}

bool QPECategoryGroup::remove( const QString &label )
{
    return FALSE;
}

bool QPECategoryGroup::rename( const QString &oldLabel, const QString &newLabel )
{
    return FALSE;
}

bool QPECategoryGroup::contains(const QString &label) const
{
    return FALSE;
}

QStringList QPECategoryGroup::categories() const
{
    return QStringList();
}
