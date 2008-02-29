/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
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
