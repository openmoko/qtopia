/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qfile.h>
#include <qtopia/pim/qpimrecord.h>
#include <qtopianamespace.h>
#include <qsettings.h>
#include <quniqueid.h>
#include <qfileinfo.h>
#include <qtextdocument.h>
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
#include "qpimxmlio_p.h"
#include <qglobal.h>
#include <qmessagebox.h>

static const uint smallestSize = 256;
static const char *UidKey = "Uid"; // no tr
static const char *CategoriesKey = "Categories"; // no tr

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
        } while (strncmp(hsearch, needle, qMin(hLen - (hsearch - haystack), nLen)) != 0);
        hsearch--;
    }
    return ((char *)hsearch);
}

QPimXmlIO::QPimXmlIO( )
{
}

QPimXmlIO::~QPimXmlIO()
{
}

bool QPimXmlIO::lockDataFile(QFile& file) {
#if !defined (Q_WS_QWS)
    Q_UNUSED( file );
    return true;
#else
    bool result;
    if (file.openMode() == QIODevice::ReadOnly)
        result = Qtopia::lockFile(file, 0);
    else
        result = Qtopia::lockFile(file, Qtopia::LockWrite);
#if !defined (Q_WS_WIN32) && defined (DEBUG)
   // "work" over NFS
    if (!result && errno == ENOLCK)
        result = true;
#endif

    return result;
#endif
}

bool QPimXmlIO::unlockDataFile(QFile& file) {
#if !defined (Q_WS_QWS)
    Q_UNUSED( file );
    return true;
#else
    bool result = Qtopia::unlockFile(file);
#if !defined (Q_WS_WIN32) && defined (DEBUG)
   // "work" over NFS
    if (!result && errno == ENOLCK)
        result = true;
#endif
    return result;
#endif
}


/**
 * Loads the record data into the internal list
 */
bool QPimXmlIO::loadData()
{
    QString normalFile = dataFilename();
    QString journalFile = journalFilename();

    if ( QFile::exists( normalFile ) ) {
        if (!loadFile( normalFile ))
            return false;;
    }

    if ( QFile::exists( journalFile ) ) {
        if (!loadFile( journalFile ))
            return false;
    }
    return true;
}

static QString decodeEntities( const QString& str )
{
    QString tmp( str );
    int pos = -1;
    while ( (pos = tmp.indexOf( "&", pos +1 ) ) != -1 ) {
        if ( tmp.mid(pos,5)=="&amp;" )
            tmp.replace( pos, 5, "&" );
        else if ( tmp.mid(pos,4)=="&lt;" )
            tmp.replace( pos, 4, "<" );
        else if ( tmp.mid(pos,4)=="&gt;" )
            tmp.replace( pos, 4, ">" );
        else if ( tmp.mid(pos,6)=="&quot;" )
            tmp.replace( pos, 6, "\"" );
    }
    return tmp;
}

bool QPimXmlIO::loadFile(const QString &filename)
{
  // With the exception of the few lines at the bottom of this
  // method, this code was stolen from 1.5 todo.

    QFile f( filename );
    if ( !f.open(QIODevice::ReadOnly) )
        return false;
    if (!lockDataFile(f)) {
      f.close();
      return false;
    }

    QByteArray ba = f.readAll();
    unlockDataFile(f);
    f.close();
    char* dt = ba.data();
    int len = ba.size();

    QMap<QString, QString> recMap;
    if ( len > 0 ) {
        int i = 0;
        char *point;
        const char *collectionString = recordStart();
        // Shouldn't use strstr, dt probably isn't terminated with a \0.
        // in very rare cases could lead to corruption of data.
        while ( ( point = strstrlen( dt+i, len - i, collectionString , strlen(collectionString)) ) != NULL ) {
            int action = ACTION_ADD;
            i = point - dt;
            QPimRecord *rec = createRecord();

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
                QString attr = QString::fromUtf8( dt+i, j-i);

                i = ++j; // skip =

                // find the start of quotes
                while ( i < len && dt[i] != '"' )
                    ++i;
                j = ++i;

                bool haveUtf = false;
                bool haveEnt = false;
                while ( j < len && dt[j] != '"' ) {
                    if ( ((unsigned char)dt[j]) > 0x7f )
                        haveUtf = true;
                    if ( dt[j] == '&' )
                        haveEnt = true;
                    ++j;
                }
                if ( i == j ) {
                    // empty value
                    i = j + 1;
                    continue;
                }

                // XXX I hope this is right...
                QString str;
                if ( haveUtf )
                    str = QString::fromUtf8( dt+i, j-i );
                else
                    str = QString::fromLatin1( dt+i, j-1 );
                i = j + 1;

                // XXX need to do via set fields, not set fields
                if ( haveEnt )
                    str = decodeEntities( str );

                if ( attr.toLower() == "action" ) // No tr
                {
                    action = str.toInt();
                } else {
                    recMap.insert(attr, str);
                }

            }
            setFields(rec, recMap);
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

    return true;
}

void QPimXmlIO::setFields(QPimRecord *rec, const QMap<QString, QString> &recMap) const
{
    QMap<QString, QString>::ConstIterator it;
    for (it = recMap.begin(); it != recMap.end(); ++it) {
        QString attr = it.key();
        QString val = it.value();
        if (attr == UidKey) {
            // xml is read only in 4.0...
            rec->setUid( val.toInt() );
        } else if (attr == CategoriesKey) {
            rec->setCategories( QPimXmlIO::idsFromString( val ) );
        } else {
            rec->setCustomField(attr, val );
        }
    }
}

bool QPimXmlIO::saveData(const QList<QPimRecord*> &m_Records) {
    QFile masterFile(dataFilename());
    if (!masterFile.open(QIODevice::ReadWrite))
        return false;
    if (!lockDataFile(masterFile)) {
        masterFile.close();
        return false;
    }

    QString strNewFile = Qtopia::tempName( dataFilename() );
    QFile f( strNewFile );
    if ( !f.open( QIODevice::WriteOnly ) )
        return false;
    if (!lockDataFile(f)) {
        f.close();
        return false;
    }

    QString buf(listStart());

    QByteArray str;
    int total_written;

    QListIterator<QPimRecord*> it(m_Records);
    foreach (QPimRecord *rec, m_Records) {
        buf += recordStart();

        buf += recordToXml(rec);

        buf += " />\n";
        str = buf.toUtf8();
        total_written = f.write( str );
        if ( total_written != int(str.length()) ) {
            f.close();
            QFile::remove( strNewFile );
            return false;
        }
        buf = "";
    }

    buf += listEnd();

    str = buf.toUtf8();
    total_written = f.write( str );
    if ( total_written != int(str.length()) ) {
        f.close();
        QFile::remove( strNewFile );
        return false;
    }
    unlockDataFile(f);
    f.close();

    // Free up the master file briefly..
    unlockDataFile(masterFile);
    masterFile.close();

    QFile::rename( strNewFile, dataFilename() );

    // remove the journal
    QFile::remove( journalFilename() );

    QFileInfo fileInfo(dataFilename());
    setLastDataReadTime(fileInfo.lastModified());

    return true;
}

void QPimXmlIO::updateJournal(const QPimRecord &rec, journal_action action)
{
    bool success = true;
    QFile f( journalFilename() );
    if ( !f.open(QIODevice::WriteOnly|QIODevice::Append) )
        success = false;
    else {
        QString buf;
        QString str;
        buf = recordStart();
        buf += recordToXml(&rec);
        buf += " action=\"" + QString::number( (int)action ) + "\" "; // No tr
        buf += "/>\n";
        QByteArray cstr = buf.toUtf8();
        int count = f.write( cstr );
        if ( count != cstr.length() )
            success = false;
        f.close();
    }
    if ( !success )
        QMessageBox::information( 0, QObject::tr( "Out of space" ),
                QObject::tr("<qt>Device full.  Some changes may not be saved.</qt>"));
}

bool QPimXmlIO::isDataCurrent() const
{
  QFileInfo fileInfo(dataFilename());
  if (fileInfo.exists() && lastDataReadTime() != fileInfo.lastModified())
      return false;

  // Don't  Check journal file, would have got this data from QCop.
  //QFileInfo fileInfoJ(journalFilename());
  //if (fileInfoJ.exists() && lastDataReadTime() != fileInfoJ.lastModified())
      //return false;
  return true;
}


QString QPimXmlIO::recordToXml(const QPimRecord *p) const
{
    QMap<QString,QString> data = fields(p);

    QString out;
    QMap<QString, QString>::Iterator it;
    for (it = data.begin(); it != data.end(); ++it) {
        QString k = it.key();
        QString v = it.value();
        if (!k.isEmpty() && !v.isEmpty()) {
            out += k;
            out += "=\"" + Qt::escape(v) + "\" ";
        }
    }
    return out;
}

QMap<QString, QString> QPimXmlIO::fields(const QPimRecord *p) const
{
    QMap<QString,QString> m;
    m.insert(UidKey, p->uid().toString());
    m.insert(CategoriesKey, idsToString(p->categories()));
    QMap<QString, QString>::ConstIterator it;

    QMap<QString, QString> cm = p->customFields();
    for (it = cm.begin(); it != cm.end(); ++it)
        m.insert(it.key(), it.value());

    return m;
}

QUniqueId QPimXmlIO::generateUid() const
{
    QUniqueIdGenerator g(mScope);

    return g.createUniqueId();
}

// Helper functions.

void QPimXmlIO::assignNewUid( QPimRecord *r) const
{
    ((QPimRecord *)r)->setUid( generateUid() );
}

void QPimXmlIO::setUid( QPimRecord &r, const QUniqueId & id) const
{
    // TODO re-examin xml id usage.
    QUniqueIdGenerator g(mScope);
    if (id.isNull()) {
        ((QPimRecord &)r).setUid(g.createUniqueId());
    } else {
        ((QPimRecord &)r).setUid(id);
    }
}

// convenience methods provided for loading and saving to xml
QString QPimXmlIO::idsToString( const QList<QString> &cats )
{
    return QStringList(cats).join(";");
}

// convenience methods provided for loading and saving to xml
QList<QString> QPimXmlIO::idsFromString( const QString &str )
{
    return str.split(";");
}

QString QPimXmlIO::dateToXml( const QDate &d )
{
    if ( d.isNull() || !d.isValid() )
        return QString();

    // ISO format in year, month, day; e.g. 20021231
    QString year = QString::number( d.year() );
    QString month = QString::number( d.month() );
    month = month.rightJustified( 2, '0' );
    QString day = QString::number( d.day() );
    day = day.rightJustified( 2, '0' );

    return year + month + day;
}

QDate QPimXmlIO::xmlToDate( const QString &s )
{
    QDate date;

    if ( s.isEmpty() )
        return date;

    int year = s.mid(0, 4).toInt();
    int month = s.mid(4,2).toInt();
    int day = s.mid(6,2).toInt();

    // do some quick sanity checking
    if ( year < 1900 || year > 3000 ) {
        qWarning( "QContact year is not in range");
        return date;
    }
    if ( month < 0 || month > 12 ) {
        qWarning( "QContact month is not in range");
        return date;
    }
    if ( day < 0 || day > 31 ) {
        qWarning( "QContact day is not in range");
        return date;
    }

    date.setYMD( year, month, day );
    if ( !date.isValid() ) {
        qWarning( "QContact date is not valid");
        return QDate();
    }

    return date;
}

QString QPimXmlIO::customToXml( const QPimRecord *r ) const
{
    QString buf(" ");
    QMap<QString, QString>::ConstIterator cit;
    const QMap<QString, QString> cmap = r->customFields();
    for (cit = cmap.begin(); cit != cmap.end(); ++cit) {
        buf += cit.key();
        buf += "=\"";
        buf += Qt::escape(cit.value());
        buf += "\" ";
    }
    return buf;
}
