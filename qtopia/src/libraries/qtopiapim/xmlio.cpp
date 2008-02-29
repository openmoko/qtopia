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

#define QTOPIA_INTERNAL_FILEOPERATIONS
#include <qfile.h>
#include <qasciidict.h>
#include <qtopia/pim/pimrecord.h>
#include <qtopia/stringutil.h>
#include <qtopia/config.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <errno.h>
#ifndef Q_WS_WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include "xmlio_p.h"
#include <qtopia/global.h>
#include <qmessagebox.h>

static const uint smallestSize = 256;

char *strstrlen(const char *haystack, int hLen, const char* needle, int nLen)
{
    char needleChar;
    char haystackChar;
    if (!needle || !haystack || !hLen || !nLen)
	return 0;

    const char* hsearch = haystack;

    if ((needleChar = *needle++) != 0) {
	nLen--; //(to make up for needle++)
	do {
	    do {
		if ((haystackChar = *hsearch++) == 0)
		    return (0);
		if (hsearch >= haystack + hLen)
		    return (0);
	    } while (haystackChar != needleChar);
	} while (strncmp(hsearch, needle, QMIN(hLen - (hsearch - haystack), nLen)) != 0);
	hsearch--;
    }
    return ((char *)hsearch);
}


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
    if ((int)pos != -1 && pos < nextindex ) {
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
	resize((size()+1) << 1);
    }
    insert(nextindex, i);
    nextindex++;
    dirty = TRUE;
}

void PimVector::clear()
{
    QVector<PimRecord>::clear(); // 0 out elements.
    nextindex = 0;
    dirty = FALSE;
}

void PimVector::sort()
{
    if (dirty)
	QVector<PimRecord>::sort();
    dirty = FALSE;
}


PimXmlIO::PimXmlIO( const QMap<int,QCString> &m1, const QMap<QCString,int> &m2 )
    :  keyToIdentifier(m1), identifierToKey(m2)
{
}

PimXmlIO::~PimXmlIO()
{
}

bool PimXmlIO::lockDataFile(QFile& file) {
#if !defined (_WS_QWS_)
    Q_UNUSED( file );
    return TRUE;
#else
    bool result;
    if (file.mode() == IO_ReadOnly)
	result = Global::lockFile(file, 0);
    else
	result = Global::lockFile(file, Global::LockWrite);
#if !defined (Q_WS_WIN32) && defined (DEBUG)
   // "work" over NFS
    if (!result && errno == ENOLCK)
    	result = TRUE;
#endif

    return result;
#endif
}

bool PimXmlIO::unlockDataFile(QFile& file) {
#if !defined (_WS_QWS_)
    Q_UNUSED( file );
    return TRUE;
#else
    bool result = Global::unlockFile(file);
#if !defined (Q_WS_WIN32) && defined (DEBUG)
   // "work" over NFS
    if (!result && errno == ENOLCK)
	result = TRUE;
#endif
    return result;
#endif
}


/**
 * Loads the record data into the internal list
 */
bool PimXmlIO::loadData()
{
    QString normalFile = dataFilename();
    QString journalFile = journalFilename();

    if ( QFile::exists( normalFile ) ) {
	if (!loadFile( normalFile ))
	    return FALSE;;
    }

    if ( QFile::exists( journalFile ) ) {
	if (!loadFile( journalFile ))
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

    if ( len > 0 ) {
	int i = 0;
	char *point;
	const char *collectionString = recordStart();
	QMap<int,QString> recMap;
	// Shouldn't use strstr, dt probably isn't terminated with a \0.
	// in very rare cases could lead to corruption of data.
	while ( ( point = strstrlen( dt+i, len - i, collectionString , strlen(collectionString)) ) != NULL ) {
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

		if ( attr.lower() == "action" ) // No tr
		{
		    action = value.toInt();
		} else {
		    QMap<QCString,int>::ConstIterator it = identifierToKey.find( attr );
		    if ( it != identifierToKey.end() ) {
			if ( *it == PimRecord::UID_ID ) {
			    setUid( *rec, uuidFromInt( str.toInt() ) );
			} else {
			    recMap[*it] = str;
			}
			//rec->setField( *it, str );
		    } else {
			rec->setCustomField(attr, str );
			//qDebug("setCustomfield, cannot handle right now");
			//qDebug("attr %s, str %s", attr.data(), str.data() );
		    }
		}

	    }
	    rec->setFields( recMap );
	    recMap.clear();
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

    QString strNewFile = Global::tempName( dataFilename() );
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

    Global::renameFile( strNewFile, dataFilename() );

    // remove the journal
    QFile::remove( journalFilename() );

    QFileInfo fileInfo(dataFilename());
    setLastDataReadTime(fileInfo.lastModified());

    return TRUE;
}

void PimXmlIO::updateJournal(const PimRecord &rec, journal_action action)
{
    bool success = TRUE;
    QFile f( journalFilename() );
    if ( !f.open(IO_WriteOnly|IO_Append) )
	success = FALSE;
    else {
	QString buf;
	QCString str;
	buf = recordStart();
	buf += recordToXml(&rec);
	buf += " action=\"" + QString::number( (int)action ) + "\" "; // No tr
	buf += "/>\n";
	QCString cstr = buf.utf8();
	uint count = f.writeBlock( cstr.data(), cstr.length() );
	if ( count != cstr.length() )
	    success = FALSE;
	f.close();
    }
    if ( !success )
	QMessageBox::information( 0, QObject::tr( "Out of space" ),
		QObject::tr("<qt>Device full.  Some changes may not be saved.</qt>"));
}

bool PimXmlIO::isDataCurrent() const
{
  QFileInfo fileInfo(dataFilename());
  if (fileInfo.exists() && lastDataReadTime() != fileInfo.lastModified())
      return FALSE;

  // Don't  Check journal file, would have got this data from QCop.
  //QFileInfo fileInfoJ(journalFilename());
  //if (fileInfoJ.exists() && lastDataReadTime() != fileInfoJ.lastModified())
      //return FALSE;
  return TRUE;
}

QString PimXmlIO::recordToXml(const PimRecord *p)
{
    QMap<int,QString> data = p->fields();

    QString out;
    for ( QMap<int, QString>::ConstIterator fit = data.begin();
	    fit != data.end(); ++fit ) {
	const QString &value = fit.data();
	int key = fit.key();

	QMap<int,QCString>::ConstIterator keyId = keyToIdentifier.find( key );
	if ( !value.isEmpty() && keyId != keyToIdentifier.end() ) {
	    QString k = *keyId;
	    if ( !k.isEmpty() ) { // else custom
		out += k;
		out += "=\"" + Qtopia::escapeString(value) + "\" ";
	    }
	}
    }

    out += customToXml( p );

    return out;
}

static int nextId = 0;
#ifdef QTOPIA_DESKTOP
    static int sign = 1;
#else
    static int sign = -1;
#endif

int PimXmlIO::generateUid()
{

    Config pimConfig( "pim" );
    pimConfig.setGroup("uid");
    if ( nextId == 0 ) {
	// load from file
	nextId = pimConfig.readNumEntry( "nextId", 0 );
	if ( !nextId )
	    nextId = sign * (int) ::time(NULL);
    }
    int res = nextId;
    nextId += sign;
    pimConfig.writeEntry( "nextId", nextId );
    return res;
}

// Helper functions.

void PimXmlIO::assignNewUid( PimRecord *r) const
{
    ((PrRecord *)r)->setUid( uuidFromInt( generateUid() ) );
}

void PimXmlIO::setUid( PimRecord &r, const QUuid &u) const
{
    int id = uuidToInt(u);
    if (nextId == 0)
	generateUid(); // init uid.
#ifdef QTOPIA_DESKTOP
    if (id >= nextId && id > 0) {
#else
    if (id <= nextId && id < 0) {
#endif
	nextId = id + sign;
	Config pimConfig( "pim" );
	pimConfig.setGroup("uid");
	pimConfig.writeEntry( "nextId", nextId );
    }
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

QString PimXmlIO::dateToXml( const QDate &d )
{
    if ( d.isNull() || !d.isValid() )
	return QString::null;

    // ISO format in year, month, day; e.g. 20021231
    QString year = QString::number( d.year() );
    QString month = QString::number( d.month() );
    month = month.rightJustify( 2, '0' );
    QString day = QString::number( d.day() );
    day = day.rightJustify( 2, '0' );

    QString str = year + month + day;
    //qDebug( "\tPimContact dateToStr = %s", str.latin1() );
    return str;
}

QDate PimXmlIO::xmlToDate( const QString &s )
{
    QDate date;

    if ( s.isEmpty() )
	return date;

    int year = s.mid(0, 4).toInt();
    int month = s.mid(4,2).toInt();
    int day = s.mid(6,2).toInt();

    // do some quick sanity checking
    if ( year < 1900 || year > 3000 ) {
	qWarning( "PimContact year is not in range");
	return date;
    }
    if ( month < 0 || month > 12 ) {
	qWarning( "PimContact month is not in range");
	return date;
    }
    if ( day < 0 || day > 31 ) {
	qWarning( "PimContact day is not in range");
	return date;
    }

    date.setYMD( year, month, day );
    if ( !date.isValid() ) {
	qWarning( "PimContact date is not valid");
	return QDate();
    }

    return date;
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
