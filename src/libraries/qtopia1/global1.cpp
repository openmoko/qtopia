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

#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/storage.h>
#include <qtopia/mimetype.h>
#include <qtopia/version.h>
#include <qtopia/custom.h>
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
#include <uuid/uuid.h>
}
#endif

#include <errno.h>

/*!
  Returns TRUE if the user regards their week as starting on Monday.
  Returns FALSE if the user regards their week as starting on Sunday.
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
*/

QString Global::homeDirPath()
{
    QString r = QDir::homeDirPath();
#ifdef QTOPIA_DESKTOP
    r += "/.palmtopcenter/";
#endif
    return r;
}

/*!
  Renames the file \a from to \a to.
  Returns TRUE if the operation is successful.
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
*/

QString Global::journalFileName(const QString &filename)
{
    QString r = QDir::homeDirPath();
#ifndef Q_WS_QWS
    r += "/.palmtopcenter/temp";
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
*/

QString Global::version()
{
    return QPE_VERSION;
}

/*!
  Returns the device architecture string. This is a sequence
  of identifiers separated by "/", from most general to most
  specific (eg. "IBM/PC").
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
*/

QString Global::deviceId()
{
    Config cfg( QPEApplication::qpeDir()+"/etc/Security.conf", Config::File );
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
*/

QString Global::ownerName()
{
    Config cfg( QPEApplication::qpeDir()+"/etc/Security.conf", Config::File );
    cfg.setGroup("Sync");
    QString r=cfg.readEntry("ownername");
    return r;
}

/*!
  \internal
*/

Global::Command* Global::builtinCommands()
{
    return builtin;
}

/*!
  \internal
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

