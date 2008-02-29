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
#ifndef QWS_DATABASE
#define QWS_DATABASE

#include <qtopia/qpeglobal.h>
#include <qstring.h>
#include <qobject.h>
#include <qintdict.h>

class QFile;
class QDataStream;

class QWSDatabasePrivate;
class QWSDatabase;

class QTOPIA_EXPORT QWSDatabaseIndex : public QObject
{
    Q_OBJECT

public:
    QWSDatabaseIndex(const QString& indname, QWSDatabase* parent);
    virtual ~QWSDatabaseIndex();

    virtual int count() const=0;

protected:
    virtual bool flushIndex();

    virtual void insert(const QByteArray&, uint pos)=0;
    virtual void remove(const QByteArray&, uint pos)=0;

    virtual void reindex()=0;
    virtual void insertDirect(const QByteArray& r, uint pos)=0;

    void setDirty( bool dirty );
    bool isDirty() const { return dirty; }
    
private:
    friend class QWSDatabase;
    friend class QWSDatabasePrivate;

protected:
    QWSDatabase* db;
    QFile* idxfile;
private:
    bool dirty;
};

class QTOPIA_EXPORT QWSDatabaseOrderedSelection : public QWSDatabaseIndex
{
    Q_OBJECT

public:
    QWSDatabaseOrderedSelection(const QString& indname, QWSDatabase* parent);
    virtual ~QWSDatabaseOrderedSelection();

    int count() const;

    int find(const QByteArray&, uint pos=-1) const;
    QByteArray at(uint index) const;
    int add(const QByteArray&);
    bool removeAt(uint index);
    uint position(uint index) const;

    virtual bool select(const QByteArray&, bool insert);
    virtual int compare(const QByteArray& a, const QByteArray& b) const;

protected:
    bool flushIndex();

    virtual void insert(const QByteArray&, uint pos);
    virtual void remove(const QByteArray&, uint pos);

    void reindex();
    void insertDirect(const QByteArray& r, uint pos);

private:
    void removeDirect(uint r);
    void resize(uint newsize);
    void replace(uint pos, uint value) ;

private:
    int compare(const void *a, const void *b) const;
    int compareKey(const void *a, const void *b) const;
    static int externCompare(const void *a, const void*b);
    static int externKeyCompare(const void *a, const void*b);
    uint capacity;
    uint n;
    uint *index;
    int last;
};

class QTOPIA_EXPORT QWSDatabase : public QObject
{
    Q_OBJECT

public:
    QWSDatabase(const QString& dbname, QObject* parent=0, const char* name=0);
    ~QWSDatabase();

    QString databaseName() const { return dbname; }

    void add( const QByteArray& );

    // For indexes only
    void addIndex(QWSDatabaseIndex*);
    void removeIndex(QWSDatabaseIndex*);
    void removeRecordAt( uint, QWSDatabaseIndex* skip = 0 );
    void reinsertDirect(QWSDatabaseIndex* i);

    QString attributeName( int attr ) const;
    int attribute( const QString &attributeName ) const;

    bool indexing() const;
    void setIndexing(bool);

    void addAttribute( int,  const QString &attributeName );
    void removeAttribute( int );
    // no edit attribute (currently) on purpose

    QByteArray atOffset(uint p) const;

protected:

    void reindex();
  

    uint writeRecord(const QByteArray &r);
    QByteArray readRecord(uint offset) const;
    bool eraseRecord(uint offset, bool ignore_type = TRUE);

    uint dirtCount() const;

    void writeHeader();
    bool readHeader();

private:

    uint readUint() const;
    bool writeUint(uint);

    bool attemptSetBlockType(unsigned char);
    bool attemptWriteBlock(uint offset, const char *, uint size);
    bool attemptWriteUint(uint offset, uint v);
    void updateDirtCount();

    /* returns 0=retry, 1=cancel */
    int recoverableError() const; 
    void unrecoverableError() const;

    void recover(QWSDatabaseIndex *i, bool rfl, bool truncate);

    QWSDatabasePrivate *d;

    uint dirty;

    QString dbname;
    QFile *dbfile;
    QFile *hdrfile;
};


#endif
