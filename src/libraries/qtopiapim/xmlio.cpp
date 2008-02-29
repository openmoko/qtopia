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
** Contact info\@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qfile.h>
#include <qasciidict.h>
#include <qtopia/pim/pimrecord.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/stringutil.h>
#include <qfileinfo.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include "xmlio_p.h"


static const uint smallestSize = 256;

PimVector::PimVector()
        : QVector<PimRecord>(smallestSize), nextindex(0), dirty(FALSE)
{}

PimVector::PimVector(uint s)
        : QVector<PimRecord>(QMIN(s, smallestSize)), nextindex(0), dirty(FALSE)
{}

PimVector::~PimVector() {}

int PimVector::compareItems(Item d1, Item d2) 
{
    PimRecord *pt1 = (PimRecord *)d1;
    PimRecord *pt2 = (PimRecord *)d2;

    if (pt1->uid() < pt2->uid())
	return -1;
    
    if (pt2->uid() < pt1->uid())
	return 1;

    return 0;
}

void PimVector::remove(PimRecord *i)
{
    uint pos = findRef(i);
    if ((int)pos != -1) {
	dirty = TRUE;
	nextindex--;
	if (pos != nextindex) {
	    insert(pos, take(nextindex));
	} else {
	    take(pos);
	}
	if (nextindex > smallestSize && nextindex << 2 < size()) {
	    resize(size() >> 1);
	}
    }
}

void PimVector::append(PimRecord *i)
{
    if (nextindex == size()) {
	resize(size() << 1);
    }
    insert(nextindex, i);
    nextindex++;
    dirty = TRUE;
}

void PimVector::clear()
{
    QVector<PimRecord>::clear(); // 0 out elements.
    nextindex = 0;
    resize(smallestSize);
    dirty = FALSE;
}

void PimVector::sort() 
{
    if (dirty)
	QVector<PimRecord>::sort();
    dirty = FALSE;
}


PimXmlIO::PimXmlIO(): maxId(1)
{
}

PimXmlIO::~PimXmlIO() 
{
}

bool PimXmlIO::lockDataFile(QFile& file) {
    struct flock fileLock;
    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    fileLock.l_len = 1;
    if (file.mode() == IO_ReadOnly) {
        fileLock.l_type = F_RDLCK;
    }
    else {
      fileLock.l_type = F_WRLCK;
    }

    int r = fcntl(file.handle(), F_SETLK, &fileLock);
    if ( r != -1
#ifdef DEBUG
   // "work" over NFS
	    || errno == ENOLCK
#endif
    ) {
	return true;
    }
    return false;
}

bool PimXmlIO::unlockDataFile(QFile& file) {
  struct flock fileLock;
  fileLock.l_whence = SEEK_SET;
  fileLock.l_start = 0;
  fileLock.l_len = 1; 
  fileLock.l_type = F_UNLCK;
  bool success = (fcntl(file.handle(), F_SETLK, &fileLock) != -1);
#ifdef DEBUG
   // "work" over NFS
   if (!success && errno == ENOLCK)
	success = TRUE;
#endif
  return success;
}


/**
 * Loads the record data into the internal list
 */
bool PimXmlIO::loadData() 
{
    if (!loadFile(dataFilename()))
	return FALSE;;

    if ( QFile::exists( journalFilename() ) ) {
	if (!loadFile( journalFilename() ))
	    return FALSE;
    }
    return TRUE;
}

bool PimXmlIO::loadFile(const QString &filename)
{
  // With the exception of the few lines at the bottom of this
  // method, this code was stolen from 1.5 todo.

    QFile f( filename );
    if ( !f.open(IO_ReadOnly) )
	return FALSE;
    if (!lockDataFile(f)) {
      f.close();
      return FALSE;
    }

    QByteArray ba = f.readAll();
    unlockDataFile(f);
    f.close();
    char* dt = ba.data();
    int len = ba.size();

    int i = 0;
    char *point;
    const char *collectionString = recordStart();
    while ( ( point = strstr( dt+i, collectionString ) ) != NULL ) {
	int action = ACTION_ADD;
	i = point - dt;
	PimRecord *rec = createRecord();

	i += strlen(collectionString);

	while( 1 ) {
	    while ( i < len && (dt[i] == ' ' || dt[i] == '\n' || dt[i] == '\r') )
		++i;
	    if ( i >= len-2 || (dt[i] == '/' && dt[i+1] == '>') )
		break;

	    // we have another attribute, read it.
	    int j = i;
	    while ( j < len && dt[j] != '=' )
		++j;
	    QCString attr( dt+i, j-i+1);

	    i = ++j; // skip =

	    // find the start of quotes
	    while ( i < len && dt[i] != '"' )
		++i;
	    j = ++i;

	    bool haveUtf = FALSE;
	    bool haveEnt = FALSE;
	    while ( j < len && dt[j] != '"' ) {
		if ( ((unsigned char)dt[j]) > 0x7f )
		    haveUtf = TRUE;
		if ( dt[j] == '&' )
		    haveEnt = TRUE;
		++j;
	    }
	    if ( i == j ) {
		// empty value
		i = j + 1;
		continue;
	    }

	    QCString value( dt+i, j-i+1 );
	    i = j + 1;

	    QString str = (haveUtf ? QString::fromUtf8( value )
		    : QString::fromLatin1( value ) );
	    if ( haveEnt )
		str = Qtopia::plainString( str );

	    if (attr == "Action" || attr == "action")
	    {
		action = value.toInt();
	    } else {
		assignField(rec, attr, str);
	    }

	}
	switch( action ) {
	case ACTION_ADD:
		internalAddRecord(rec);
		break;
	    case ACTION_REMOVE:
		internalRemoveRecord(rec);
		break;
	    case ACTION_REPLACE:
		internalUpdateRecord(rec);
		break;
	}
    }

    // Record when the file was last modified so we know if
    // someone messes with it while we are looking at it
    QFileInfo fileInfo(dataFilename());
    setLastDataReadTime(fileInfo.lastModified());
    return TRUE;
}

bool PimXmlIO::saveData(const QList<PimRecord> &m_Records) {
    QFile masterFile(dataFilename());
    if (!masterFile.open(IO_ReadWrite))
	return FALSE;
    if (!lockDataFile(masterFile)) {
	masterFile.close();
	return FALSE;
    }

    QString strNewFile = dataFilename() + ".new";
    QFile f( strNewFile );
    if ( !f.open( IO_WriteOnly|IO_Raw ) )
	return FALSE;
    if (!lockDataFile(f)) {
	f.close();
	return FALSE;
    }

    QString buf(listStart());

    QCString str;
    int total_written;

    QListIterator<PimRecord> it(m_Records);
    for (; it.current() ; ++it) {
	PimRecord *rec  = it.current();
	buf += recordStart();

	buf += recordToXml(rec);

	buf += " />\n";
	str = buf.utf8();
	total_written = f.writeBlock( str.data(), str.length() );
	if ( total_written != int(str.length()) ) {
	    f.close();
	    QFile::remove( strNewFile );
	    return FALSE;
	}
	buf = "";
    }

    buf += listEnd();

    str = buf.utf8();
    total_written = f.writeBlock( str.data(), str.length() );
    if ( total_written != int(str.length()) ) {
	f.close();
	QFile::remove( strNewFile );
	return FALSE;
    }
    unlockDataFile(f);
    f.close();

    // Free up the master file briefly..
    unlockDataFile(masterFile);
    masterFile.close();

    // now do the rename
    if ( ::rename( strNewFile, dataFilename() ) < 0 )
	qWarning( "problem renaming file %s to %s errno %d",
		strNewFile.latin1(), dataFilename().latin1(), errno );

    // remove the journal
    QFile::remove( journalFilename() );

    QFileInfo fileInfo(dataFilename());
    setLastDataReadTime(fileInfo.lastModified());

    return TRUE;
}

void PimXmlIO::updateJournal(const PimRecord &rec, journal_action action)
{
    QFile f( journalFilename() );
    if ( !f.open(IO_WriteOnly|IO_Append) )
	return;
    QString buf;
    QCString str;
    buf = recordStart();
    buf += recordToXml(&rec);
    buf += " action=\"" + QString::number( (int)action ) + "\" ";
    buf += "/>\n";
    QCString cstr = buf.utf8();
    f.writeBlock( cstr.data(), cstr.length() );
    f.close();
}

bool PimXmlIO::isDataCurrent() const
{
  bool current = true;
  QFileInfo fileInfo(dataFilename());
  if (lastDataReadTime() < fileInfo.lastModified()) {
    qDebug("Data is not current due to timestamp.");
    current = false;
  }
  return current;
}

// Helper functions.

void PimXmlIO::assignNewUid( PimRecord *r) const
{

#ifdef PALMTOPCENTER
    int id = maxId;
#else
    // we use the sign bit on the pda
    int id = -maxId;
#endif
    qDebug("maxId now %d", maxId);

    ((PrRecord *)r)->setUid(uuidFromInt( id ));
    // built on the abundance of integers theory (see Warwick).
    maxId++;
}

void PimXmlIO::setUid( PimRecord &r, const QUuid &u) const
{
    int id = uuidToInt(u);
#ifdef PALMTOPCENTER
    // only look at the > 0's
    if (id > 0 && id >= maxId)
	maxId = id + 1;
#else
    // we use the sign bit on the pda
    // only look at the < 0's
    if (id < 0 && -id >= maxId)
	maxId = -id + 1;
#endif
    ((PrRecord &)r).setUid(u);
}

QUuid PimXmlIO::uid( const PimRecord &r) const
{
    return r.uid();
}


// convenience methods provided for loading and saving to xml
QString PimXmlIO::idsToString( const QArray<int> &cats )
{
    QString str;
    for ( uint i = 0; i < cats.size(); i++ )
	if ( i == 0 )
	    str = QString::number( cats[int(i)] );
	else
	    str += ";" + QString::number( cats[int(i)] );

    return str;
}

// convenience methods provided for loading and saving to xml
QArray<int> PimXmlIO::idsFromString( const QString &str )
{
    QStringList catStrs = QStringList::split( ";", str );
    QArray<int> cats( catStrs.count() );
    uint i = 0;
    for ( QStringList::ConstIterator it = catStrs.begin();
	    it != catStrs.end(); ++it ) {
	cats[int(i)] = (*it).toInt();
	i++;
    }
    return cats;
}

QUuid PimXmlIO::uuidFromInt( int u )
{
    QUuid id;
    id.data1 = u;
    return id;
}

int PimXmlIO::uuidToInt( const QUuid &u )
{
    return u.data1;
}

QString PimXmlIO::customToXml( const PimRecord *r ) const
{
    QString buf(" ");
    QMap<QString, QString> &customMap = ((PrRecord *)r)->customRef();
    for ( QMap<QString, QString>::ConstIterator cit = customMap.begin();
	    cit != customMap.end(); ++cit) {
	buf += cit.key();
	buf += "=\"";
	buf += Qtopia::escapeString(cit.data());
	buf += "\" ";
    }
    return buf;
}
