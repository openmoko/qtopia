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

#include "database.h"
#include <qtopia/categories.h>
#include "qwsdatabase_p.h"
#include <qintdict.h>

/* TODO
Fix Uids
Testing
*/
//#define DEBUG
#undef DEBUG

//using namespace qpe;

class DatabaseSelection : public QWSDatabaseOrderedSelection
{
public:
    DatabaseSelection( DatabaseView *view, const QString& indname, QWSDatabase* parent)
	: QWSDatabaseOrderedSelection( indname, parent ), mView( view )
    {
    }	
	
    bool select(const QByteArray&a, bool ) 
    {
	Record r( a );
	return !mView->filter( r );
    }

    virtual int compare(const QByteArray& a, const QByteArray& b) const
    {
	Record r1(a);
	Record rs(b);
	return mView->compare( a, b );
    }
    DatabaseView *mView;
};


/* NOT IN 1.5.0
  \class Database
  \brief The Database class keeps an unordered list of Record on disk.

  The Database class keeps an unordered list of Records on disk.  It 
  also ensure file consistency in the face of both sudden killing of the 
  program and the possiblity of running out of disk space.  There is a 
  moderate attempt to keep space wastage down but it is not very aggressive 
  in reclaiming lost space.

  Records placed in the database a written and flushed to disk before the
  database returns.  If for whatever reason the operation fails, it should
  alter the accessability of the remaining data.  

  Also if there is a gap in the Database big enough to accomidate the added 
  data, then the data will be inserted into this gap rather than at the end 
  of the file.
*/

/*! 
  Constructs a database object for the application \a appname. \a cats must
  be a valid pointer to a categories object. The \a parent and \a name
  parameters are standard QObject parameters.
*/
Database::Database( const QString &appname, Categories *cats,
	      QObject * parent, const char *name  )
    : modified( FALSE ), categories( cats )
{
    db = new QWSDatabase( appname, parent, name );
    mDefaultView = new DatabaseDefaultView( this, "def" );
}

/*!
  deletes the database object and closes the associated files
*/
Database::~Database()
{
}

/*!
  \fn Database::defaultView() const
  
  Returns a pointer to the default View for the database
  
  \sa DatabaseView
*/

/*!
  Inserts the record \a rec into the database. If the record exists, its data  will be
  replaced by the data in \a rec.
*/
bool Database::insert( const Record &rec )
{
#ifdef DEBUG
    qDebug("Database::insert");
#endif
    modified = TRUE;
    if ( rec.mIndex != -1 ) {
// 	qDebug("replacing record" );
	db->removeRecordAt( rec.mIndex );
    } 
//     else
// 	qDebug("adding new record");
    db->add( rec.data );
    return TRUE;
}

/*!
  Removes the record \a rec from the database.
*/
bool Database::remove( const Record &rec )
{
#ifdef DEBUG
    qDebug("database::remove");
#endif
    modified = TRUE;
    if ( rec.mIndex != -1 ) 
	db->removeRecordAt( rec.mIndex );
    return TRUE;
}

/*!
  returns the name of the database.
*/
QString Database::name() const
{
    return db->databaseName();
}


class DatabaseViewPrivate 
{
public:
    QIntDict<DatabaseSelection> catidx;
};

/* NOT IN 1.5.0
  \class DatabaseView
  
  \brief The DatabaseView class is a general view onto a Database.
  
  A DatabaseView object describes a general view onto a Database. The view can specify
  non standard filtering or sorting orders.
  
  A database view is aware of categories and handles them internally. This makes general category 
  handling in the application simple and seamless.
*/

/*!
  Constructs a view named \a name, of the database \a parent. You can have multiple views of a
  database. Each view requires some space for indexing to allow for fast access.
*/
DatabaseView::DatabaseView( Database *parent, const QString &name ) 
    : QObject( parent ), mName( name ), mDatabase( parent )
{
    d = new DatabaseViewPrivate;
    createSelections( *mDatabase->categories );

#if 0
    // ### do we need something like this???
    if ( mDatabase->modified ) {
	// need to recreate indices
	DatabaseSelection *idx;
	for (QIntDictIterator<DatabaseSelection> it(d->catidx); (idx=it.current()); ++it)
		idx->reindex();
    }
#endif
    connect( mDatabase->categories, SIGNAL( categoriesChanged(const Categories&) ), 
	     this, SLOT( categoriesChanged(const Categories&) ) );
}

/*!
  Returns the number of records in the current view that belong into the category \a catid.
  A \a catid of 0 means the general category "all".
*/
uint DatabaseView::count( int catid ) const
{
    DatabaseSelection *sel = d->catidx[catid];
#ifdef DEBUG
    qDebug("DatabaseView::count( catid %d ) = %d", catid, sel ? sel->count() : 0 );
#endif
    return sel ? sel->count() : 0;
}

/*!
  Returns the total count of records in this view.
*/
uint DatabaseView::countAll() const
{
    return count( 0 );
}

/*!
  Returns the Record \a numInCat in Category \a catid.
*/
Record DatabaseView::find( int catid, uint numInCat ) const
{
#ifdef DEBUG
    qDebug("DatabaseView::find: looking up record %d in cat %d", numInCat, catid);
#endif
    DatabaseSelection *sel = d->catidx[catid];
    if ( !sel ) return Record();
#ifdef DEBUG
    qDebug("DatabaseView::find: found record");
#endif
    QByteArray r = sel->at( numInCat ); 
    Record rec( r );
    rec.mIndex = sel->position( numInCat );
    return rec;
}

/*!
  Returns the record \a numInAll for the "all" category.
*/
Record DatabaseView::find( uint numInAll ) const
{
    return find( 0, numInAll );
}

/*!
  Removes the Record at \a numInAll from the database.
*/
bool DatabaseView::remove( uint numInAll )
{
    return d->catidx[0]->removeAt( numInAll );
}

/*!
  Removes the Record \a numInCat from the category \a catid.
  If \a catid is 0 ("all"), the Record will be removed from the database.
*/
bool DatabaseView::remove( int catid, uint numInCat )
{
#ifdef DEBUG
    qDebug("DatabaseView::remove( catid %d, numInCat %d", catid, numInCat );
#endif
    DatabaseSelection *sel = d->catidx[catid];
    if ( !sel ) return FALSE;
    QByteArray r = sel->at( numInCat );
    if ( catid == 0 )
	sel->removeAt( numInCat );
    else {
	Record rec( r );
	QArray<int> cats = rec.categories();
	int num = cats.count();
	bool found = FALSE;
	for( int i = 0; i < num; i++ )
	    if ( (found && i < num-1) || cats[i] == catid ) {
		cats[i] = cats[i+1];
		found = TRUE;
	    }
	if ( !found )
	    qWarning("DatabaseView::remove: category not in record!");
	else
	    cats.truncate( cats.size() - 1 );
	rec.setCategories( cats );
	mDatabase->insert( rec );
    }
    return TRUE;
}

/*!
  Inserts the record \a r into the category \a cat.
*/
int DatabaseView::insert( int cat, const Record &r )
{
    DatabaseSelection *sel = d->catidx[cat];
    if ( !sel )
	sel = d->catidx[0];
    Record rec( r );
    QArray<int> cats = rec.categories();
    int num = cats.count();
    bool found = FALSE;
    for( int i = 0; i < num; i++ )
	if ( (found && i < num-1) || cats[i] == cat ) {
	    cats[i] = cats[i+1];
	    // don't need to do anything
	    // ## shoudn't be 0
	    return 0;
	}
    cats.resize( cats.size() + 1 );
    cats[num] = cat;
    
    if ( r.mIndex != -1 ) {
	mDatabase->db->removeRecordAt( r.mIndex );
    }
    return sel->add( r.data );
}

/*!
  \internal
*/
void DatabaseView::categoriesChanged( const Categories &c )
{
    createSelections( c );
}

/*!
  \internal
*/
void DatabaseView::createSelections( const Categories &categories )
{
#ifdef DEBUG
    qDebug("DatabaseView::createSelections" );
#endif
    qFatal("DatabaseView::createSelections not implemented" );
/* doesn't compile anymore
    QArray<int> categoryList = categories.ids( mDatabase->name() );
    
    // create one index for each category
    d->catidx.insert(0, new DatabaseSelection(this, mName + "." + QString::number( 0 ), 
						      mDatabase->db) );
    int num = categoryList.count();
    for ( int i = 0; i < num; i++ )
	d->catidx.insert(categoryList[i], 
		      new DatabaseSelection(this, mName + "." + QString::number( categoryList[i] ), 
						      mDatabase->db) );
*/
}


/*!
  Constructs the default view named \a name, of database \a parent.
  This is the view containing all items (no filtering) in their default sort order.
*/
DatabaseDefaultView::DatabaseDefaultView( Database *parent, const QString &name ) 
    : DatabaseView( parent, name )
{
}


/*!\reimp
 */
int DatabaseDefaultView::compare( const Record &a, const Record &b )
{
    return a.compare( b );
}
