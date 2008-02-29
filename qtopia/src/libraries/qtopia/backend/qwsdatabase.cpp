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
#include "qwsdatabase_p.h"

#include <qtopia/global.h>

#include <qlist.h>
#include <qmap.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qmessagebox.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

/* TODO
   * Use more efficient data structures, e.g. QMap for the sort order could be
     considered overkill.
   * more complete performance tests.
*/

const QWSDatabaseOrderedSelection* comparisonidx;

// Normal bsearch(), but returns the element to insert before
// in order to maintain sorting. Only returns NULL if must append.
static void* bsearch2(const void *key, const void *base, size_t nmemb,
              size_t size, int (*compar)(const void *, const void *))
{
    size_t l, u, idx;
    const void *p=0;
    int comparison=0;

    l = 0;
    u = nmemb;
    while (l < u)
    {
	idx = (l + u) / 2;
	p = (void*) (((const char *) base) + (idx * size));
	comparison = (*compar)(key, p);
	if (comparison < 0)
	    u = idx;
	else if (comparison > 0)
	    l = idx + 1;
	else
	    return (void*) p;
    }

    if ( comparison > 0) {
	if ( l == nmemb )
	    return 0;
	else
	    return (void*)((char*)p+size);
    } else {
	return (void*)p;
    }
}

class RecordWithPos {
public:
    RecordWithPos(const QByteArray& r, int p=-1) : record(r), pos(p) {}

    const QByteArray& record;
    int pos;
};


QWSDatabaseIndex::QWSDatabaseIndex(const QString& indname, QWSDatabase* d) :
    QObject(d),
    db(d)
{
    dirty = FALSE;
    idxfile = new QFile(Global::applicationFileName("Database", db->databaseName() + "." + indname + ".idx")); // No tr
    if (idxfile->open(IO_ReadWrite)) {
	if ( idxfile->size() == 0 ) {
	    setDirty( TRUE ); 
	}
    }
    db->addIndex(this);
}

QWSDatabaseOrderedSelection::QWSDatabaseOrderedSelection(const QString& indname, QWSDatabase* d) :
    QWSDatabaseIndex(indname,d),
    capacity(64), n(0)
{
    index = (uint *)malloc(sizeof(uint) * capacity);

    if ( isDirty() ) {
	reindex();
    } else {
	uint c = 0;
	idxfile->flush();
	idxfile->at(0);
	QDataStream ds(idxfile);
	ds >> c; // dirty flag
	if ( c != 0x0 ) {
	    qDebug("index dirty!!!");
	    reindex();
	    setDirty( TRUE );
	} else {
	    ds >> c;
	    resize(c);

	    int pb = 0;
	    while(c-- > 0) {
		uint current = 0;
		int pos = idxfile->at();
		ds >> current;
		index[pb] = current;
		pb++;
	    }
	}
    }
}

void QWSDatabaseIndex::setDirty( bool d )
{
    if( dirty != d && idxfile->isOpen() ) {
	int pos = idxfile->at();
	idxfile->at( 0 );
	QDataStream ds( idxfile );
	uint invalid = 0xffffffff;
	ds << (int) (d ? invalid : 0x0);
	idxfile->flush();
	idxfile->at(pos);
    }
    dirty = d;
}


QWSDatabaseIndex::~QWSDatabaseIndex()
{
    flushIndex();
    setDirty( FALSE );
    if ( idxfile->isOpen() ) {
	idxfile->close();
    }
    db->removeIndex(this);
}

QWSDatabaseOrderedSelection::~QWSDatabaseOrderedSelection()
{
    flushIndex();
    free(index);
}

bool QWSDatabaseIndex::flushIndex()
{
    if ( !isDirty() )
	return FALSE;
    if ( idxfile->isOpen() )
	idxfile->close();
    idxfile->open(IO_WriteOnly);
    setDirty( TRUE );
    return TRUE;
}

bool QWSDatabaseOrderedSelection::flushIndex()
{
    if ( !QWSDatabaseIndex::flushIndex() )
	return FALSE;

    idxfile->at( 4 ); // skip dirty flag
    QDataStream ds(idxfile);

    ds << n;

    for(uint i = 0; i < n; i++) {
	ds << position(i);
    }
    setDirty( FALSE );

    return TRUE;
}

void QWSDatabaseOrderedSelection::resize(uint newsize)
{
    /* powers of two, later swap *=,/= for shifts. */
    uint c = capacity;
    n = newsize;
    if (newsize > 64) {
	while (newsize > c) 
	    c *= 2;

	/* the shrink test */
	uint doublenew = newsize * 2;
	while (doublenew < c)
	    c /= 2;

	if (c != capacity) {
	    /* we need to resize and memcpy */

	    uint* ind = (uint *)malloc(sizeof(uint) * c);
	    if (!ind) {
		qWarning("could not allocate memory for index");
		return;
	    }
	    if(index) {
		if (c > capacity) 
		    memcpy(ind, index, capacity * sizeof(uint));
		else 
		    memcpy(ind, index, c * sizeof(uint));
		free(index);
	    }
	    capacity = c;
	    index = ind;
	}
    }
}

int QWSDatabaseOrderedSelection::count() const
{
    return n;
}

QByteArray QWSDatabaseOrderedSelection::at(uint r) const
{
    return db->atOffset(position(r));
}

int QWSDatabaseOrderedSelection::find(const QByteArray &r, uint pos) const 
{
    comparisonidx = this;
    RecordWithPos rp(r,pos);
    const uint* p = (const uint*)bsearch(&rp, index, n, sizeof(*index), externKeyCompare);
    comparisonidx = 0;
    return p ? p-index : -1;
}


uint QWSDatabaseOrderedSelection::position(uint pos) const
{
    if (pos >= n)
	return 0;
    return index[pos];
}

void QWSDatabaseOrderedSelection::replace(uint pos, uint value) 
{
    if (pos >= n)
	return ;
    index[pos] = value;
    setDirty( TRUE );
}

bool QWSDatabaseOrderedSelection::removeAt(uint r)
{
    if (r >= n)
	return FALSE;
    int pos = index[r];
    // removeDirect(r); - can't since we want to see it in remove(r,pos)
    db->removeRecordAt(pos,0); // can't skip, we want to see it
    return TRUE;
}

void QWSDatabaseOrderedSelection::removeDirect(uint r)
{
    setDirty( TRUE );
    // + 1 as because although it is valid, it does not affect the memmove
    if (r + 1 != n) {
	memmove(index + r,
		index + (r + 1),
		(n - (r + 1)) * sizeof(uint));
    }

    resize(n-1);
}

void QWSDatabaseOrderedSelection::remove(const QByteArray& r, uint pos)
{
    if ( select(r,FALSE) ) {
	int i=find(r,pos);
	if ( i>=0 )
	    removeDirect(i);
    }
}

int QWSDatabaseOrderedSelection::add(const QByteArray& r)
{
    last = -1;
    db->add(r);
    return last;
}

void QWSDatabaseOrderedSelection::insert(const QByteArray& r, uint pos)
{
    if ( select(r,TRUE) ) {
	resize(n+1);
	comparisonidx = this;
	RecordWithPos rp(r,pos);
	uint* p = (uint*)bsearch2(&rp, index, n-1, sizeof(*index), externKeyCompare);
	comparisonidx = 0;
	if ( p ) {
	    int i = p-index;
	    memmove(index + i + 1, index + i, (n - i - 1) * sizeof(uint));
	    index[last=i] = pos;
	} else {
	    index[last=n-1] = pos;
	}
	setDirty( TRUE );
    }
}

void QWSDatabaseOrderedSelection::insertDirect(const QByteArray& r, uint pos)
{
    if ( select(r,TRUE) ) {
	resize(n+1);
	index[n-1] = pos;
    }
}

void QWSDatabaseOrderedSelection::reindex()
{
    qDebug("reindexing");
    n = 0;
    db->reinsertDirect(this);
    comparisonidx = this;
    qsort(index, n, sizeof(uint), externCompare);
    comparisonidx = 0;
    setDirty( TRUE );
}

int QWSDatabaseOrderedSelection::externCompare(const void *a, const void*b)
{
    return comparisonidx->compare(a, b);
}

int QWSDatabaseOrderedSelection::externKeyCompare(const void *a, const void*b)
{
    return comparisonidx->compareKey(a, b);
}

struct FreeList {
    uint size;
    uint start;
    uint count;
};

static const uint number_lists = 4;
static const uint smallest_list = 32;

class QWSDatabasePrivate
{
public:
    QWSDatabasePrivate() : indexed(TRUE)
    { 
	keys.setAutoDelete(TRUE);

	// initialize the free lists.
	for (uint i = 0; i < number_lists; i++) 
	{
	    freelists[i].size = smallest_list << i;
	    freelists[i].start = 0;
	    freelists[i].count = 0;
	}
    }

    ~QWSDatabasePrivate() 
    {
    }

    uint dbend;

    bool indexed;

    void indexify(const QByteArray& r, uint pos)
    {
	if ( indexed ) {
	    QWSDatabaseIndex* idx;
	    for (QListIterator<QWSDatabaseIndex> it(indices); (idx=it.current()); ++it)
		idx->insert(r,pos);
	}
    }

    void unindexify(uint pos, QWSDatabaseIndex* skip, QWSDatabase* db)
    {
	// remove from all indexes except "skip".

	if ( indexed ) {
	    if ( indices.count() &&
		    (!skip || indices.count() != 1 || skip != indices.first()) )
	    {
		QByteArray r = db->atOffset(pos);
		QWSDatabaseIndex* idx;
		for (QListIterator<QWSDatabaseIndex> it(indices); (idx=it.current()); ++it)
		    if ( idx != skip )
			idx->remove(r,pos);
	    }
	}
    }

    void reindex()
    {
	if (indexed) {
	    QWSDatabaseIndex* idx;
	    for (QListIterator<QWSDatabaseIndex> it(indices); (idx=it.current()); ++it)
		idx->reindex();
	}
    }

    QIntDict<QString> keys;

    QList<QWSDatabaseIndex> indices;

    FreeList freelists[number_lists];
};

bool QWSDatabaseOrderedSelection::select(const QByteArray&, bool)
{
    return TRUE;
}

int QWSDatabaseOrderedSelection::compare(const QByteArray&, const QByteArray&) const
{
    return 0;
}

int QWSDatabaseOrderedSelection::compare(const void *a, const void *b) const
{
    const uint pa = *((uint *)a);
    const uint pb = *((uint *)b);
    QByteArray first = db->atOffset(pa);
    QByteArray second = db->atOffset(pb);
    int o = compare(first,second);
    if ( o ) return o;
    return pa-pb;
}

int QWSDatabaseOrderedSelection::compareKey(const void *a, const void *b) const
{
    RecordWithPos* key = (RecordWithPos*)a;
    int pos = *(int *)b;
    QByteArray value = db->atOffset(pos);
    int order = compare(key->record, value);
    if ( !order && key->pos != -1 )
	return key->pos-pos;
    return order;
}


const unsigned char type_invalid = 0xff; // this file is wrecked, 
					 // any data past this point is
					 // suspect.
const unsigned char type_unknown = 0xfe; // the size field following is
					 // correct, but the data 
					 // stored is indeterminate.

const unsigned char type_record = 0x01;  // this is a record
const unsigned char type_free = 0x02;    // this is an element of a free
					 // list (during regen, ignore the
				 	 // next pointer).

/*!
  \internal
  \class QWSDatabase
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
QWSDatabase::QWSDatabase(const QString& nm, QObject* parent, const char* name) :
    QObject(parent, name)
{
    dbname = nm;
    QString base = Global::applicationFileName("Database", nm); // No tr
    dbfile = new QFile(base + ".dat");
    hdrfile = new QFile(base + ".hdr");

    d = 0;

    if (!dbfile->open(IO_ReadWrite)) {
	return;
    }
    dbfile->at(0);
    if (dbfile->size() == 0) {
	dirty = 0;
	if (!writeUint(dirty)) {
	    // couln't even write 4 bytes
	    while (recoverableError() == 0) {
		if (writeUint(dirty))
		    break;
	    }
	}
    } else {
	dirty = readUint();
    }

    d = new QWSDatabasePrivate;

    if (!readHeader()) {
	// we don't have enough info to continue, TODO
    }
}

QWSDatabase::~QWSDatabase()
{
    dbfile->close();
    writeHeader();

    if (d)
	delete d;
}

/*!
  Adds \a r to the database, and if currently index, also inserts it
  into any active indexes.
  */
void QWSDatabase::add(const QByteArray& r) 
{
    if (!d)
	return;
    
    uint index = writeRecord(r);
    if (index)
	d->indexify(r,index);
}

/*! 
  Scans the database for valid records, and for each valid record inserts
  it into the index \a i.
  */ 
void QWSDatabase::reinsertDirect(QWSDatabaseIndex* i)
{
    recover(i, FALSE, FALSE);
}

/*! 
  Scans the database, and tries to reconstruct records, freelists, and
  file integrity.  Unless diskspace runs out or the process is killed 
  the will be no uknown or invalid records remaining when this function 
  completes.
 */
void QWSDatabase::recover(QWSDatabaseIndex* i, 
	bool recoverFreeLists, bool truncate)
{
    uint offset = sizeof(uint);
    bool stillErrors = FALSE;
    dbfile->at(offset);

    if (recoverFreeLists) {
	for (uint ind = 0; ind < number_lists; ind++) {
	    d->freelists[ind].start = 0;
	    d->freelists[ind].count = 0;
	}
    }

    unsigned char test = 0;
    while(!dbfile->atEnd()) {
	test = dbfile->getch();
	uint size = readUint();
	bool failed = FALSE;
	switch(test) {
	    case type_record:
		if (i)
		    i->insertDirect(readRecord(offset), offset);
		break;
	    case type_free:
	    case type_unknown:
		if (recoverFreeLists) {
		    if (!eraseRecord(offset, TRUE))
			stillErrors = TRUE;
		}
		break;
	    default: // covers test_invalid.
		// something has gone wrong, stop reading
		if (truncate) {
		    Global::truncateFile(dbfile->handle(), offset);
		    dbfile->flush();
		    d->dbend = offset;
		} else {
		    qWarning("WARNING, data inconsistency found, "
			    "invalid element found while parsing records");
		}
		failed = TRUE;
		break;
	}
	if (failed)
	    break;
	offset += size + 1 + sizeof(uint);
	dbfile->at(offset);
    }
}

/*!
  Adds a new index \a i to the database.
*/
void QWSDatabase::addIndex(QWSDatabaseIndex* i)
{
    d->indices.append(i);
}

/*!
  Removes index \a i from the database.
*/
void QWSDatabase::removeIndex(QWSDatabaseIndex* i)
{
    d->indices.removeRef(i);
}

void QWSDatabase::removeRecordAt(uint pos, QWSDatabaseIndex* skip)
{
    d->unindexify(pos,skip,this);
    if ( !skip )
	eraseRecord( pos );
}

/*!
  Returns the record at offset \a pos in the database file.
  */
QByteArray QWSDatabase::atOffset(uint pos) const
{
    return readRecord(pos);
}

/*!
  Sets indexing.  If \a b is TRUE then all elements in the database
  will be inserted into the index, the order of elements being maintained.

  Otherwise elements inserted into the database will not be added to the
  index.
*/
void QWSDatabase::setIndexing(bool b)
{
    if (d && d->indexed != b ) {
	d->indexed = b;
	if (b)
	    reindex();
    }
}

/*!
  Returns TRUE if the database is currently adding new items to the indexs.

  Otherwise returns FALSE.
*/
bool QWSDatabase::indexing() const
{
    if (d)
	return d->indexed;

    return FALSE;
}


/*!
  Returns the name of the attribute \a attr.
*/
QString QWSDatabase::attributeName(int attr) const
{
    if (!d)
	return QString::null;
    QString* s = d->keys[attr];
    return s ? *s : QString::null;
}

/*!
  Returns the index of the attribute \a attr.
*/
int QWSDatabase::attribute(const QString &attr) const
{
    if (!d)
	return -1;

    QIntDictIterator<QString> it( d->keys ); // See QIntDictIterator
    for ( ; it.current(); ++it )
	if(!qstrcmp(*(it.current()), attr))
	    return it.currentKey();

    return -1;
}

/*!
  Adds a new attribute to the database with name \a attr at position
  \a id.
*/
void QWSDatabase::addAttribute(int id, const QString &attr)
{
    if(!d)
	return;

    d->keys.replace(id, new QString(attr));
}

/*!
  \internal
  used to read a uint from the database.
*/
uint QWSDatabase::readUint() const
{
    uint v = 0;
    v += dbfile->getch();
    v <<= 8;
    v += dbfile->getch();
    v <<= 8;
    v += dbfile->getch();
    v <<= 8;
    v += dbfile->getch();
    return v;
}

/* return false on failure */
bool QWSDatabase::writeUint(uint v)
{
    uint mask = 0x000000FF;
    bool success = TRUE;

    if (dbfile->putch(v >> 24 & mask) == -1)
	success = FALSE;
    if (dbfile->putch(v >> 16 & mask) == -1)
	success = FALSE;
    if (dbfile->putch(v >> 8 & mask) == -1)
	success = FALSE;
    if (dbfile->putch(v & mask) == -1)
	success = FALSE;

    return success;
}

/*!
  \internal
  Attempts to write \a value at \a offset in the database file.  If an error
  occurs the database will pop a warning querying the user if they want
  to retry or cancel the write.
*/
bool QWSDatabase::attemptWriteUint(uint offset, uint value)
{
    while (1) {
	dbfile->resetStatus();
	dbfile->at(offset);
	writeUint(value);
	dbfile->flush();

	if (dbfile->status() == IO_Ok)
	    break;
	if (recoverableError() == 1)
	    return FALSE;
    }
    return TRUE;
}

/*!
  \internal
  Attempts to write \a size bytes of \a data at \a offset in the 
  database file.  If an error occurs the database will pop a warning 
  querying the user if they want to retry or cancel the write.
*/
bool QWSDatabase::attemptWriteBlock(uint offset, const char *data, uint size) 
{
    dbfile->resetStatus();
    while (1) {
	dbfile->at(offset);
	dbfile->writeBlock(data, size);
	dbfile->flush();

	if (dbfile->status() == IO_Ok)
	    break;
	if (recoverableError() == 1)
	    return FALSE;
	dbfile->resetStatus();
    }
    return TRUE;
}

/*!
  \internal
  This is a very important function.  It attempts (just once) to
  write the block type \a t into the 
  database file.  If an error occurs the database will pop a warning 
  indicate that an unrecoverable error has occurred.  The operation cannot
  be completed.
*/
bool QWSDatabase::attemptSetBlockType(unsigned char t) 
{
    dbfile->resetStatus();
    dbfile->putch(t);
    dbfile->flush();

    if (dbfile->status() != IO_Ok) {
	unrecoverableError(); // tell the user
	return FALSE;
    }

    return TRUE;
}
void QWSDatabase::updateDirtCount()
{
    // now update the dirtCount;
    dirty++;
    dbfile->at(0);
    writeUint(dirty);
    dbfile->flush();

    // this is at the start of the file.  IF this fails, I don't 
    // think we can recover.
}

/*!
  Attempts to write record \a r into the database.  If successful, returns 
  the offset where the record was written.  Otherwise returns 0.
  
  \sa readRecord(), eraseRecord
 */
uint QWSDatabase::writeRecord(const QByteArray &r)
{
    QByteArray ba = r;

    uint real_size = ba.size() + 1 + sizeof(uint);
    /* ba now contains the data (and records it size) */
    /* first check the free lists */

    uint list_num = 0;
    do {
	if (
		(d->freelists[list_num].count > 0)
		&&
		(d->freelists[list_num].size >= real_size)
	   )
	    break;
	list_num++;
    } while(list_num < number_lists);
    if (list_num == number_lists)
	list_num--;
    // list num is now the list with size just larger than real_size.
    // (or is the last list

    uint offset;
    uint size = 0;
    Q_UINT32 next = 0;
    if (d->freelists[list_num].count > 0) {
	// search the free list for the right size (first fit).
	offset = d->freelists[list_num].start;
	uint last = 0;
	uint count = d->freelists[list_num].count;
	bool found_gap = FALSE;
	while(count--) {
	    dbfile->at(offset);
	    
	    unsigned char test = 0;
	    test = dbfile->getch();
	    if(test != type_free) {
		qWarning("WARNING, data inconsistency found, free list element wasn't");
		//recordError(test);  // future
		break; // no free list found
	    }
	    size = readUint();
	    next = readUint();

	    if (size >= ba.size()) {
		if (last) {

		    dbfile->at(last);
		    if (!attemptSetBlockType(type_unknown))
			return 0; // no harm done, yet

		    if (!attemptWriteUint(last + 1 + sizeof(uint), next))
			return 0;

		    dbfile->at(last);
		    if (!attemptSetBlockType(type_free))
			return 0; // harm done, but can fix later

		} else {
		    d->freelists[list_num].start = next;
		}
		ba.resize(size); // so erase gets the right amount of space
		d->freelists[list_num].count -= 1;
		found_gap = TRUE;
		break;
	    } else {
		last = offset;
		offset = next;
	    }
	}

	if (found_gap) {
	    dbfile->at(offset);
	    if (!attemptSetBlockType(type_unknown))
		return 0; 

	    if (!attemptWriteBlock(offset + 1 + sizeof(uint),
			ba.data(), size))
		return 0;  // harm done but we will recover

	    // mark as record
	    dbfile->at(offset);
	    if (!attemptSetBlockType(type_record))
		return 0; // record is there... but we can't touch it.

	    updateDirtCount();

	    return offset;
	}
    }

    // failed to find offset for whatever reason 
    offset = d->dbend;
    d->dbend = offset + real_size;
    
    dbfile->at(offset);
    if (!attemptSetBlockType(type_invalid))
	return 0;

    if (!attemptWriteUint(offset + 1, ba.size()))
	return 0;
    if (!attemptWriteBlock(offset + 1 + sizeof(uint), ba.data(), ba.size()))
	return 0;

    // mark as record.
    dbfile->at(offset);
    if (!attemptSetBlockType(type_record))
	return 0;

    updateDirtCount();

    return offset;
}

/*!
  Attempts to read a record from the database.  If successful, returns 
  the record at \a offset.  Otherwise returns a the default record
  empty ByteArray.
  
  \sa writeRecord(), eraseRecord
*/
QByteArray QWSDatabase::readRecord(uint offset) const
{
    QByteArray ba;

    dbfile->at(offset);

    char test = dbfile->getch();
    if (test != type_record) {
	//qWarning("WARNING, data inconsistency found, record element wasn't a record");
	return QByteArray();  // no record here
    }

    uint size = readUint();
    ba.resize(size);
    dbfile->readBlock(ba.data(), size);

    return QByteArray(ba);
}

/*!
  Attempts to erase whatever is at \a offset in the database file.  If
  \a ignore_type is FALSE then will fail if \a offset is not the beginning
  of a record.

  This function will return FALSE if there was an error in erasing the
  record.  This doesn't indicate that the record wasn't erased, but rather
  that it is indeterminate whether the record was erased.  If the record
  was successfully erased the function returns TRUE.
  
  \sa writeRecord(), readRecord
*/
bool QWSDatabase::eraseRecord(uint offset, bool ignore_type)
{
    if(offset > d->dbend)
	return FALSE;

    dbfile->at(offset);

    unsigned char test = dbfile->getch();
    if (test == type_invalid || (!ignore_type && test != type_record)) {
	qWarning("WARNING, data inconsistency found record marked" 
		" for deletion not a record.");
	return FALSE; // um, shouldn't erase something that isn't a record.
    }

    QDataStream ds(dbfile);
    uint orig_size = readUint();

    // logic in writeRecord attempts to assure this will always be a
    // multiple of smallest_list;
    uint erase_size = orig_size + 1 + sizeof(uint);
    /* do what is needed to mark this record as erased */

    if (offset + erase_size >= d->dbend) {
	Global::truncateFile(dbfile->handle(), offset);
	dbfile->flush();
	d->dbend = offset;
	return TRUE;
    }

    uint list_num = 0;
    do {
	if (
		(d->freelists[list_num].count > 0)
		&&
		(d->freelists[list_num].size >= erase_size)
	   )
	    break;
	list_num++;
    } while(list_num < number_lists);
    if (list_num == number_lists)
	list_num--;
    // list num is now the list with size just larger than real_size.
    // (or is the last list

    dbfile->at(offset);
    if (!attemptSetBlockType(type_unknown))
	return FALSE; // we haven't modified anything yet.

    if (!attemptWriteUint(offset + 1 + sizeof(uint), 
		d->freelists[list_num].start))
	return FALSE; // its an unknonw block now, can fix later.

    // we assume this succeeds, no failure if it doesn't
    // now 0 out remaining space
    for (uint i = 0; i < (orig_size - 4); i++)
	dbfile->putch(0);
    dbfile->flush();

    dbfile->at(offset);
    if (!attemptSetBlockType(type_free))
	return FALSE; // we haven't modified anything yet.

    d->freelists[list_num].count += 1;
    d->freelists[list_num].start = offset;

    updateDirtCount();

    return TRUE;
}

void QWSDatabase::reindex()
{
    d->reindex();
}

/*!
  Returns the dirt count of the database.  This is useful for classes
  that store separate data to be sure they are still in sync with the database.
*/
uint QWSDatabase::dirtCount() const
{
    return dirty;
}

void QWSDatabase::writeHeader()
{
    hdrfile->open(IO_WriteOnly);
    QDataStream ds(hdrfile);

    ds << dirty;
    ds << d->dbend;
    uchar key_count = d->keys.count();
    ds << key_count;

    QIntDictIterator<QString> it( d->keys ); // See QIntDictIterator
    for ( ; it.current(); ++it ) {
	uchar key = it.currentKey();
	const char *name = it.current()->utf8();
	ds << key;
	ds << name;
    }

    // the free lists
    uint ui;
    for(ui = 0; ui < number_lists; ui++) {
	ds << d->freelists[ui].start;
	ds << d->freelists[ui].count;
    }

    hdrfile->close();
}

bool QWSDatabase::readHeader()
{
    if (!d)
	return FALSE;

    if (!hdrfile->exists()) {
	d->dbend = 4; // after all first 4 bytes are a dirtcount
	// everything else is initialized already, or at least should be.
	writeHeader();
    } else {
	hdrfile->open(IO_ReadOnly);
	QDataStream ds(hdrfile);

	uint tmp_dirty;
	ds >> tmp_dirty;
	if (tmp_dirty != dirty) {
	    /* some problem has occurred in storing the header.
	       Attempt to recover the free lists */
	    recover(0, TRUE, TRUE);
	    hdrfile->close();
	    return TRUE;
	}
	d->dbend = 0;
	ds >> d->dbend;

	uchar key_count = 0;
	ds >> key_count;

	while(key_count-- > 0) {
	    uchar key = 0;
	    char *keyName;
	    ds >> key;
	    ds >> keyName;
	    d->keys.replace(key, new QString(QString::fromUtf8(keyName)));
	}

	for(uint i = 0; i < number_lists; i++) {
	    uint start = 0;
	    uint end = 0;
	    ds >> start;
	    ds >> end;
	    d->freelists[i].start = start;
	    d->freelists[i].count = end;
	}

	hdrfile->close();
    }
    return TRUE;
}

int QWSDatabase::recoverableError() const
{
    // FIXME This message probably needs to say more to the user.
    return QMessageBox::critical(0, tr("Could not write data."), 
	    tr("Could not write data."), tr("retry"), tr("cancel"));
}

void QWSDatabase::unrecoverableError() const
{
    // FIXME This message probably needs to say more to the user.
    QMessageBox::critical(0, tr("Fatal Storage Error"), 
	    tr("Could not write data."), tr("cancel"));
}

