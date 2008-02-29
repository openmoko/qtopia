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

#ifndef QTPALMTOP_CATEGORIES_H_
#define QTPALMTOP_CATEGORIES_H_

#include <qtopia/qpeglobal.h>
#include <qstring.h>
#include <qstringlist.h>

#include <qtopia/qtopiawinexport.h>
#include <qobject.h>

class QPECategoryManagerPrivate;
class QTOPIA_EXPORT QPECategoryManager : public QObject
{
    Q_OBJECT
public:
    QPECategoryManager( QObject *parent=0, const char *name = 0 ); // from default
#ifdef QTOPIA_DESKTOP
    // doesn't make sense other than on the desktop.
    QPECategoryManager( const QCString &installLocal, QObject *parent=0, const char *name = 0 ); // from pda/deskop/sd/pda2/etc
#endif
    QPECategoryManager( const QPECategoryManager &copyFrom );

    virtual ~QPECategoryManager();

    QPECategoryManager &operator= ( const QPECategoryManager &c );

    void clearAll();
    void clearGlobal();
    void clear( const QCString &appname );

    // some categories are magic.
    // CANNOT RENAME, DELETE, OR RESCOPE these categories.
    QString businessCategory() const;
    QString personalCategory() const;
    bool isSystemCategory( const QString & ) const;

    // only checks global categories
    bool isGlobal( const QString & ) const;
    // checks all categories
    bool contains( const QString &catname ) const;
    // checks only one app or group
    bool contains( const QString &appname, const QString &catname) const;

    QStringList categories( const QCString & ) const;
    // list of only global categories
    QStringList globalCategories() const;
    // sort of a reverse.  Which apps have this category.
    QValueList<QCString> groups( const QString &category ) const;

    // implies adding the group appname
    bool addCategory( const QCString &appname, const QString &catname );
    bool addGlobalCategory( const QString &catname );

    // fails if not scoped to app, or if appName not in db. (or if system cat)
    bool removeCategory( const QCString &appName, const QString &catName);
    bool removeGlobalCategory( const QString &catName );

    bool renameCategory( const QString &appname,
			 const QString &oldName,
			 const QString &newName );
    bool renameGlobalCategory( const QString &oldName,
			       const QString &newName );

    void makeGlobal( const QString &appname, const QString &catname);
    void makeLocal( const QString &catname );

    // now for record/object interactions
    // app near same as 'table' = cat_table
    // not sure about these next 4 functions...  leave out for now
    /*
    addCategoryTable( const QCString &app );
    dropCategoryTable( const QCString &app );
    QPECategories categories( const QCString &app, const QUuid &object);
    void setCategories( const QCString &app, const QUuid &object, const QPECategories &cats );
    */

signals:
    // should just have one signal.  Refresh y'all.
    // these may have to deal with QCop's behind the scenes....
    void categoryAdded( const QCString &appname, const QString &);

    void categoryRemoved( const QCString &appname, const QString &);

    void categoryRenamed( const QCString &appname, const QString &oldname, const QString &newname );

    // no point in mentioning app, would only confuse the issue.
    void categoryMadeGlobal( const QString & );
    void categoryMadeLocal( const QString & );

private:
    void initTables();
    QPECategoryManagerPrivate *d;
};


// need to make sure right functions are virtual?
// should be app convienience.  e.g. just saves passing appname all about the place.
class QTOPIA_EXPORT QPECategoryGroup : QPECategoryManager
{
public:
    QPECategoryGroup(const QCString &); // app related
    QPECategoryGroup(); // global only
    QPECategoryGroup( const QPECategoryGroup &c );

    void clear();

    bool add( const QString &label );
    bool addGlobal( const QString &label );

    bool remove( const QString &label );

    bool rename( const QString &oldLabel, const QString &newLabel );

    bool contains(const QString &label) const;

    QStringList categories() const;
};
#endif
