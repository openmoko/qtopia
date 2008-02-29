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

#ifndef QTPALMTOP_DATABASE_H
#define QTPALMTOP_DATABASE_H

#include <qtopia/qpeglobal.h>
#include <qtopia/record.h>
#include <qtopia/record.h>

#include <qobject.h>

class QWSDatabase;
class TaskDatabaseSelection;
class Categories;
class DatabaseSelection;
class DatabaseViewPrivate;

//namespace qpe {

    class DatabaseDefaultView;

class QTOPIA_EXPORT Database : public QObject
{
    Q_OBJECT
    friend class DatabaseView;
public:
    Database( const QString &appname, Categories *cats,
	      QObject * parent = 0, const char *name = 0  );
    virtual ~Database();

    bool insert( const Record &rec );
    bool remove( const Record &rec );
    
    DatabaseDefaultView *defaultView() const { return mDefaultView; }
    
    QString name() const;
    
private:
    QWSDatabase *db;
    bool modified;
    DatabaseDefaultView *mDefaultView;
    Categories *categories;
};

class QTOPIA_EXPORT DatabaseView : public QObject
{
    Q_OBJECT
    friend class DatabaseSelection;
public:
    DatabaseView( Database *parent, const QString &name );
    
    uint count( int catid ) const;
    //uint countUnassigned() const;
    uint countAll() const;

    Record find( int catid, uint numInCat ) const;
    Record find( uint numInAll ) const;
    //Record findUnassigned( uint numInUnassigned ) const;

    bool remove( uint numInAll );
    bool remove( int catid, uint numInCat );
    
    int insert( int catidx, const Record &r );
    
protected:
    virtual bool filter( const Record & ) { return FALSE; }
    virtual int compare( const Record &a, const Record &b ) = 0;

private slots:
    void categoriesChanged( const Categories &c );

private:
    void createSelections( const Categories &c );
    
    QString mName;
    Database *mDatabase;
    DatabaseViewPrivate *d;
};

class QTOPIA_EXPORT DatabaseDefaultView : public DatabaseView
{
public:
    DatabaseDefaultView( Database *parent, const QString &name );
    
protected:
    int compare( const Record &a, const Record &b );
};

//}

#endif
