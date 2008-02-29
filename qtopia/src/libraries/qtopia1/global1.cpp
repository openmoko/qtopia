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

#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/storage.h>
#include <qtopia/mimetype.h>
#include <qtopia/version.h>
#include <qtopia/custom.h>
#ifndef QTOPIA_DESKTOP
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/qpeapplication.h>
#include <qtopia/private/contact.h>

#include <qdir.h>
#include <qfile.h>

#if defined(Q_WS_WIN32)
#include <objbase.h>
#elif defined(Q_WS_MAC)
#include <CoreFoundation/CoreFoundation.h>
#else
extern "C" {
//#include <uuid/uuid.h>   // Not usually shipped with Linux
#ifndef _UUID_UUID_H
typedef unsigned char uuid_t[16];
void uuid_generate(uuid_t out);
#endif
}
#endif

#include <errno.h>

#ifdef QTOPIA_DESKTOP
#include <qdconfig.h>
#endif

// this must be after <qdconfig.h>
#include "qpe_homeDirPath.cpp"

/*!
  Returns TRUE if the user regards their week as starting on Monday.
  Returns FALSE if the user regards their week as starting on Sunday.

  First availability: Qtopia 1.6
*/

bool Global::weekStartsOnMonday()
{
    Config config("qpe");
    config.setGroup( "Time" );
    return config.readBoolEntry("MONDAY", FALSE);
}

/*!
  Sets the day the user regards their week starting on.
  If \a v is TRUE, then the week begins on Monday.
  If \a v is FALSE, then the week begins on Sunday.

  First availability: Qtopia 1.6
*/

void Global::setWeekStartsMonday(bool v)
{
    Config config("qpe");
    config.setGroup( "Time" );
    config.writeEntry( "MONDAY", v );
}

/*!
  Returns the name of the directory to be used as the current
  users home directory.

  First availability: Qtopia 1.6
*/

QString Global::homeDirPath()
{
    return ::qpe_homeDirPath();
}

/*!
  Renames the file \a from to \a to.
  Returns TRUE if the operation is successful.

  First availability: Qtopia 1.6
*/

bool Global::renameFile( QString from, QString to )
{
    if ( from.isEmpty() || to.isEmpty() ) {
	qWarning("qtopia_renameFile emptry string(s) from '%s' to '%s'",
		 from.latin1(), to.latin1() );
	return FALSE;
    }

    // now do the rename
#ifdef Q_OS_WIN32
    if ( QFile::exists( to ) && !QFile::remove( to ) )
	qWarning("Global::renameFile failed removing %s before rename", to.latin1() );
#endif

    QDir dir;
    if ( dir.rename( from, to ) )
	return TRUE;

    qWarning( "problem renaming file using qdir::rename %s to %s",
	      from.latin1(), to.latin1() );
    //QFile::remove( from );
    // return FALSE;

    from = QDir::convertSeparators( QDir::cleanDirPath( from ) );
    to = QDir::convertSeparators( QDir::cleanDirPath( to ) );
    if ( ::rename( from, to ) != 0 ) {
 	qWarning( "::rename problem renaming file %s to %s errno %d",
	 	  from.latin1(), to.latin1(), errno );
#ifdef Q_OS_WIN32
 	qWarning("\trename error = %s", strerror(NULL) );
#endif
	return FALSE;
    }

    qDebug("worked using ::rename");
    return TRUE;
}

/*!
  Returns a filename suitable for use as a temporary file using \a fname
  as a base.

  First availability: Qtopia 1.6
*/

QString Global::tempName(const QString &fname)
{
    QString temp;
#ifndef Q_OS_WIN32
    temp = fname + ".new";
#else
    QFileInfo fileInfo( fname );
    temp = fileInfo.dirPath( TRUE ) + "/" + fileInfo.baseName() +
		 "_new." + fileInfo.extension(); // No tr
#endif
    return temp;
}

/*!
  \internal
  Sets the journal filename as \a filename.

  First availability: Qtopia 1.6
*/

QString Global::journalFileName(const QString &filename)
{
    QString r = Global::homeDirPath();
#ifndef Q_WS_QWS
    r += "/temp";
#endif
    QDir d( r );
    if ( !d.exists() )
	if ( d.mkdir(r) == false )
	    return QString::null;
    r += "/"; r += filename;

    //qDebug("Global::journalFileName = %s", r.latin1() );
    return r;
}

/*!
  Returns a Uuid with system-wide uniqueness.

  First availability: Qtopia 1.6
*/

QUuid Global::generateUuid()
{
#ifdef Q_OS_WIN32
    uuid_t uuid;
    if (UuidCreate(&uuid) == RPC_S_OK)
	return QUuid(uuid);
    else
	return QUuid();
#elif defined(Q_WS_MAC)
    CFUUIDRef uuidr;
    uuidr = CFUUIDCreate(NULL);
    CFUUIDBytes uuid = CFUUIDGetUUIDBytes(uuidr);
    QUuid id;
    memcpy(&id, &uuid, sizeof(QUuid) );
    free((char*)uuidr);
    return id;
#else
    uuid_t uuid;
    ::uuid_generate( uuid );

    QUuid id;
    memcpy(&id, &uuid, sizeof(QUuid) );
    return id;
#endif
}

/*!
  Returns TRUE if \a file is the filename of a document.

  First availability: Qtopia 1.6
*/

bool Global::isDocumentFileName(const QString& file)
{
    if ( file.right(1) == "/" )
	return FALSE;
    QString homedocs = QDir::homeDirPath() + "/Documents";
    if ( file.find(homedocs+"/")==0 )
	return TRUE;
    StorageInfo storage;
    const QList<FileSystem> &fs = storage.fileSystems();
    QListIterator<FileSystem> it ( fs );
    for ( ; it.current(); ++it ) {
	if ( (*it)->isRemovable() ) {
	    if ( file.find((*it)->path()+"/")==0 )
		return TRUE;
	}
    }
    return FALSE;
}

/*!
  Returns TRUE if \a file is the filename of an application.

  First availability: Qtopia 1.6
*/

bool Global::isAppLnkFileName(const QString& file)
{
    if ( file.right(1) == "/" )
	return FALSE;
    return file.find(MimeType::appsFolderName()+"/")==0;
}

#ifdef Q_WS_QWS

/*!
  Returns the Qtopia version string. This is of the form:
  <i>major</i>.<i>minor</i>.<i>patchlevel</i> (eg. "1.2.3"),
    possibly followed by a space and special information
    (eg. "1.2.3 beta4").

  First availability: Qtopia 1.6
*/

QString Global::version()
{
    return QPE_VERSION;
}

/*!
  Returns the device architecture string. This is a sequence
  of identifiers separated by "/", from most general to most
  specific (eg. "IBM/PC").

  First availability: Qtopia 1.6
*/

QString Global::architecture()
{
#ifndef QPE_ARCHITECTURE
# define QPE_ARCHITECTURE "Uncustomized Device" // No tr
#endif
    return QPE_ARCHITECTURE;
}

/*!
  Returns a unique ID for this device. The value can change, if
  for example, the device is reset.

  First availability: Qtopia 1.6
*/

QString Global::deviceId()
{
    Config cfg("Security");
    cfg.setGroup("Sync");
    QString r=cfg.readEntry("serverid");
    if ( r.isEmpty() ) {
	QUuid uuid = generateUuid();
	cfg.writeEntry("serverid",(r = uuid.toString()));
    }
    return r;
}

/*!
  Returns the name of the owner of the device.

  First availability: Qtopia 1.6
*/

QString Global::ownerName()
{
    Config cfg("Security");
    cfg.setGroup("Sync");
    QString r=cfg.readEntry("ownername");
    return r;
}

/*!
  \internal

  First availability: Qtopia 1.6
*/

Global::Command* Global::builtinCommands()
{
    return builtin;
}

/*!
  \internal

  First availability: Qtopia 1.6
*/

QGuardedPtr<QWidget>* Global::builtinRunning()
{
    return running;
}

#endif

static int parse64base(char *src, char *bufOut)
{
  char c, z;
  char li[4];
  int processed;

  //conversion table without table...
  for (int x = 0; x < 4; x++) {
    c = src[x];

    if ( (int) c >= 'A' && (int) c <= 'Z')
      li[x] = (int) c - (int) 'A';
    else if ( (int) c >= 'a' && (int) c <= 'z')
      li[x] = (int) c - (int) 'a' + 26;
    else if ( (int) c >= '0' && (int) c <= '9')
      li[x] = (int) c - (int) '0' + 52;
    else if (c == '+')
      li[x] = 62;
    else if (c == '/')
      li[x] = 63;
  }

  processed = 1;
  bufOut[0] = (char) li[0] & (32+16+8+4+2+1);	//mask out top 2 bits
  bufOut[0] <<= 2;
  z = li[1] >> 4;
  bufOut[0] = bufOut[0] | z;		//first byte retrived

  if (src[2] != '=') {
    bufOut[1] = (char) li[1] & (8+4+2+1);	//mask out top 4 bits
    bufOut[1] <<= 4;
    z = li[2] >> 2;
    bufOut[1] = bufOut[1] | z;		//second byte retrived
    processed++;

    if (src[3] != '=') {
      bufOut[2] = (char) li[2] & (2+1);	//mask out top 6 bits
      bufOut[2] <<= 6;
      z = li[3];
      bufOut[2] = bufOut[2] | z;	//third byte retrieved
      processed++;
    }
  }
  return processed;
}

/*!
  Decodes base64 encoded \a encoded and returns a QByteArray containing
  the decoded data.

  First availability: Qtopia 1.6
*/
QByteArray Global::decodeBase64( const QByteArray& encoded )
{
    QByteArray buffer;
    int len = encoded.size();
    buffer.resize( len * 3 / 4 + 2);
    uint bufCount = 0;
    int pos = 0, decodedCount = 0;
    char src[4];
    char *destPtr = buffer.data();

    while (pos < len && encoded[pos]) {
	decodedCount = 4;
	int x = 0;
	while ( (x < 4) && (pos < len) && encoded[pos] ) {
	    src[x] = encoded[pos];
	    pos++;
	    if (src[x] == '\r' || src[x] == '\n' || src[x] == ' ')
		x--;
	    x++;
	}
	if (x > 1) {
	    decodedCount = ::parse64base(src, destPtr);
	    destPtr += decodedCount;
	    bufCount += decodedCount;
	}
    }
    buffer.resize(bufCount);

    return buffer;
}

/*!
 Encodes \a origData using base64 mapping and returns a QString containing the
 encoded form.

  First availability: Qtopia 1.6
*/
QByteArray Global::encodeBase64(const QByteArray& origData)
{
    // follows simple algorithm from rsync code
    uchar *in = (uchar*)origData.data();

    int inbytes = origData.size();
    int outbytes = ((inbytes * 8) + 5) / 6;
    int padding = 4-outbytes%4; if ( padding == 4 ) padding = 0;

    QByteArray outbuf(outbytes+padding);
    uchar* out = (uchar*)outbuf.data();

    const char *b64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (int i = 0; i < outbytes; i++) {
        int byte = (i * 6) / 8;
        int bit = (i * 6) % 8;
        if (bit < 3) {
            if (byte < inbytes)
	        *out = (b64[(in[byte] >> (2 - bit)) & 0x3F]);
        } else {
            if (byte + 1 == inbytes) {
	        *out = (b64[(in[byte] << (bit - 2)) & 0x3F]);
            } else {
	        *out = (b64[(in[byte] << (bit - 2) |
		     in[byte + 1] >> (10 - bit)) & 0x3F]);
            }
        }
        ++out;
    }
    ASSERT(out == (uchar*)outbuf.data() + outbuf.size() - padding);
    while ( padding-- )
        *out++='=';

    return outbuf;
}

