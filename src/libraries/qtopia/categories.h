/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTPALMTOP_CATEGORIES_H
#define QTPALMTOP_CATEGORIES_H

#include <qtopia/qpeglobal.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qlistview.h>
#include <qarray.h>
#include <qtopia/private/palmtopuidgen.h>

class CategoryGroup;

class QTOPIA_EXPORT CategoryGroup
{
    friend class Categories;
public:
    CategoryGroup(): mIdLabelMap(), mLabelIdMap() { }
    CategoryGroup( const CategoryGroup &c ) :
	mIdLabelMap( c.mIdLabelMap), mLabelIdMap( c.mLabelIdMap ) { }

    void clear() { mIdLabelMap.clear(); mLabelIdMap.clear(); }

    int add( const QString &label );
    bool add( int uid, const QString &label );

    bool remove( const QString &label );
    bool remove( int uid );

    bool rename( int uid, const QString &newLabel );
    bool rename( const QString &oldLabel, const QString &newLabel );

    bool contains(int id) const;
    bool contains(const QString &label) const;

    /** Returns label associated with the uid or QString::null if
    *   not found
    */
    const QString &label(int id) const;
    /** Returns the uid associated with label or 0 if not found */
    int id(const QString &label) const;

    /** Returns a sorted list of labels */
    QStringList labels() const;

    QStringList labels( const QArray<int> &catids ) const;

    const QMap<int, QString> &idMap() const { return mIdLabelMap; }

private:
    void insert( int uid, const QString &label );
    QMap<int, QString> mIdLabelMap;
    QMap<QString, int> mLabelIdMap;

#ifndef Q_OS_WIN32
    static Qtopia::UidGen &uidGen() { return sUidGen; }
#else
	static Qtopia::UidGen &uidGen();
#endif
    static Qtopia::UidGen sUidGen;
};

#define QTOPIA_DEFINED_CATEGORYGROUP
#include <qtopia/qtopiawinexport.h>

class QTOPIA_EXPORT Categories : public QObject
{
    Q_OBJECT
public:
    Categories( QObject *parent=0, const char *name = 0 )
	: QObject( parent, name ),  mGlobalCats(), mAppCats() { }
    Categories( const Categories &copyFrom ) : QObject( copyFrom.parent() ),
	mGlobalCats( copyFrom.mGlobalCats ),
    mAppCats( copyFrom.mAppCats ) { }
    virtual ~Categories() { }

    Categories &operator= ( const Categories &c )
{ mAppCats = c.mAppCats; mGlobalCats = c.mGlobalCats; return *this; }

    void clear();

    int addCategory( const QString &appname, const QString &catname);
    int addCategory( const QString &appname, const QString &catname, int uid);
    int addGlobalCategory( const QString &catname );
    int addGlobalCategory( const QString &catname, int uid );
    bool removeCategory( const QString &appName, const QString &catName,
			 bool checkGlobal = TRUE);
    bool removeCategory( const QString &appName, int uid );
    bool removeGlobalCategory( const QString &catName );
    bool removeGlobalCategory( int uid );

    QArray<int> ids( const QString &app, const QStringList &labels) const;

    int id( const QString &app, const QString &cat ) const;
    QString label( const QString &app, int id ) const;

    enum ExtraLabels { NoExtra, AllUnfiled, AllLabel, UnfiledLabel };
    QStringList labels( const QString &app,
			bool includeGlobal = TRUE,
			ExtraLabels extra = NoExtra ) const;

    QStringList labels( const QString &app,
			const QArray<int> &catids ) const; // libqtopia

    enum DisplaySingle { ShowMulti, ShowAll, ShowFirst };

    QString displaySingle( const QString &app,
			   const QArray<int> &catids,
			   DisplaySingle display ) const;

    QStringList globalCategories() const { return mGlobalCats.labels();}

    bool renameCategory( const QString &appname,
			 const QString &oldName,
			 const QString &newName );
    bool renameGlobalCategory( const QString &oldName,
			       const QString &newName );

    void setGlobal( const QString &appname, const QString &catname,
		    bool value );
    bool isGlobal( const QString &catname ) const;


    /** Returns true if the catname is associated with any application
    */
    bool exists( const QString &catname ) const;
    bool exists( const QString &appname, const QString &catname) const;

    bool save( const QString &fname ) const;
    bool load( const QString &fname );

    // for debugging
    void dump() const;

    const QMap<QString, CategoryGroup> &appGroupMap() const{ return mAppCats; }
    const CategoryGroup &globalGroup() const { return mGlobalCats; }

#ifdef QTOPIA_DESKTOP
    static void setCategoryFileName(const QString &s);
#endif
signals:
    /** emitted if added a category;
    *  the second param is the application the category was added to
    *  or null if global
    *  the third param is the uid of the newly added category
    */
    void categoryAdded( const Categories &cats,
			const QString &appname, int uid);
    /** emitted if removed a category
    *  the second param is the application the category was removed from
    *  or null if global
    *  the third param is the uid of the removed category
    */
    void categoryRemoved( const Categories &cats, const QString &appname,
			  int uid);
    /** emitted if a category is renamed; the second param is the uid of
    *   the removed category */
    void categoryRenamed( const Categories &cats, const QString &appname,
			  int uid);

private:
    CategoryGroup mGlobalCats;
    QMap< QString, CategoryGroup > mAppCats;
};

#define QTOPIA_DEFINED_CATEGORIES
#include <qtopia/qtopiawinexport.h>

class QTOPIA_EXPORT CheckedListView : public QListView
{
public:
    void addCheckableList( const QStringList &options );
    void setChecked( const QStringList &checked );
    QStringList checked() const;
};

#define QTOPIA_DEFINED_CHECKLISTVIEW
#include <qtopia/qtopiawinexport.h>

#endif
