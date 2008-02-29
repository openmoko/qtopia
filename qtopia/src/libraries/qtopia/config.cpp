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
#define QTOPIA_INTERNAL_LANGLIST
#include "config.h"
#include "global.h"
#include "qpeapplication.h"
#include "localtr_p.h"
#include <qdir.h>
#include <qfile.h>
#include <qdict.h>
#include <qlist.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
#include <qtextcodec.h>
#endif
#include <qtextstream.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#include <sys/time.h>
#include <qfileinfo.h>
#endif

#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qapplication.h> //for translate

QString qtopia_internal_homeDirPath();

class ConfigPrivate {
public:
    ConfigPrivate() : multilang(FALSE) {}
    ConfigPrivate(const ConfigPrivate& o) :
	trfile(o.trfile),
	trcontext(o.trcontext),
	multilang(o.multilang)
    {}
    ConfigPrivate& operator=(const ConfigPrivate& o)
    {
	trfile = o.trfile;
	trcontext = o.trcontext;
	multilang = o.multilang;
	return *this;
    }

    QString trfile;
    QCString trcontext;
    bool multilang;
};

// ==========================================================================


#ifndef Q_OS_WIN32

//#define DEBUG_CONFIG_CACHE

const int CONFIG_CACHE_SIZE = 8192;
const int CONFIG_CACHE_TIMEOUT = 1000;

class ConfigData
{
public:
    ConfigData(const ConfigData& o) :
	cfg(o.cfg),
	priv(o.priv ? new ConfigPrivate(*o.priv) : 0),
	mtime(o.mtime),
	size(o.size),
	used(o.used),
        inode(o.inode)
    { }

    ConfigData& operator=(const ConfigData& o)
    {
	cfg = o.cfg;
	delete priv;
	priv = o.priv ? new ConfigPrivate(*o.priv) : 0;
	mtime = o.mtime;
	size = o.size;
	used = o.used;
        inode = o.inode;
	return *this;
    }

    ConfigData() : priv(0) {}
    ~ConfigData() { delete priv; }

    ConfigGroupMap cfg;
    ConfigPrivate *priv; // Owned by this object
    time_t mtime;
    unsigned int size;
    struct timeval used;
    Q_UINT64 inode;
};

class ConfigCache : public QObject
{
public:
    ConfigCache();

    void insert(const QString &filename, const ConfigGroupMap &cfg, const ConfigPrivate* priv);
    bool find(const QString &filename, ConfigGroupMap &cfg, ConfigPrivate*& priv);
    void remove(const QString &filename);

protected:
    void timerEvent(QTimerEvent *);

private:
    void removeLru();

    QMap<QString, ConfigData> configData;
    unsigned int totalsize;
    int tid;
};

ConfigCache::ConfigCache() : QObject(), totalsize(0), tid(0)
{
}

void ConfigCache::insert(const QString &filename, const ConfigGroupMap &cfg, const ConfigPrivate* priv)
{
    // use stat() rather than QFileInfo for speed.
    struct stat sbuf;
    stat(filename.local8Bit().data(), &sbuf);

    if (sbuf.st_size < CONFIG_CACHE_SIZE/2) {
	ConfigData data;
	data.cfg = cfg;
	data.priv = priv ? new ConfigPrivate(*priv) : 0;
	data.mtime = sbuf.st_mtime;
	data.size = sbuf.st_size;
        data.inode = sbuf.st_ino;
	gettimeofday(&data.used, 0);

	remove(filename);
	configData.insert(filename, data);

	totalsize += data.size;
#ifdef DEBUG_CONFIG_CACHE
	qDebug("++++++ insert %s", filename.latin1());
#endif
    }

    if (totalsize > (uint)CONFIG_CACHE_SIZE) {
	// We'll delay deleting anything until later.
	// This lets us grow quite large during some operations,
	// but we'll be reduced to a decent size later.
	// This works well with the main use case - app startup.
	if (!tid)
	    tid = startTimer(CONFIG_CACHE_TIMEOUT);
    }
}

bool ConfigCache::find(const QString &filename, ConfigGroupMap &cfg, ConfigPrivate*& priv)
{
    QMap<QString, ConfigData>::Iterator it = configData.find(filename);
    if (it != configData.end()) {
	ConfigData data = *it;
	// use stat() rather than QFileInfo for speed.
	struct stat sbuf;
	stat(filename.local8Bit().data(), &sbuf);

	if (data.mtime == sbuf.st_mtime && (int)data.size == sbuf.st_size
            && data.inode == sbuf.st_ino) {
	    cfg = data.cfg;
	    delete priv;
	    priv = data.priv ? new ConfigPrivate(*data.priv) : 0;
	    gettimeofday(&data.used, 0);
#ifdef DEBUG_CONFIG_CACHE
	    qDebug("******* Cache hit: %s", filename.latin1());
#endif
	    return TRUE;
	}
    }

#ifdef DEBUG_CONFIG_CACHE
    qDebug("------- Cache miss: %s", filename.latin1());
#endif

    return FALSE;
}

void ConfigCache::remove(const QString &filename)
{
    QMap<QString, ConfigData>::Iterator it = configData.find(filename);
    if (it != configData.end()) {
	totalsize -= (*it).size;
	configData.remove(it);
    }
}

void ConfigCache::timerEvent(QTimerEvent *)
{
#ifdef DEBUG_CONFIG_CACHE
    qDebug( "cache size: %d", totalsize);
#endif
    while (totalsize > (uint)CONFIG_CACHE_SIZE)
	removeLru();
    killTimer(tid);
    tid = 0;
}

void ConfigCache::removeLru()
{
    QMap<QString, ConfigData>::Iterator it = configData.begin();
    QMap<QString, ConfigData>::Iterator lru = it;
    ++it;
    for (; it != configData.end(); ++it) {
	if ((*it).used.tv_sec < (*lru).used.tv_sec ||
	    ((*it).used.tv_sec == (*lru).used.tv_sec &&
	     (*it).used.tv_usec < (*lru).used.tv_usec))
	    lru = it;
    }

#ifdef DEBUG_CONFIG_CACHE
    qDebug("Cache full, removing: %s", lru.key().latin1());
#endif
    totalsize -= (*lru).size;
    configData.remove(lru);
}

static ConfigCache *qpe_configCache = 0;

#endif /* Q_OS_WIN32 */


// ==========================================================================


/*!
  \internal
*/
QString Config::configFilename(const QString& name, Domain d)
{
    QString actualName;
    switch (d) {
	case File:
	    actualName = name;
	    break;

	case User: {
	  QString homeDirPath = ::qtopia_internal_homeDirPath();

	  QDir dir = (homeDirPath + "/Settings");
	    if ( !dir.exists() )
#ifndef Q_OS_WIN32
		mkdir(dir.path().local8Bit(),0700);
#else
	        dir.mkdir(dir.path());
#endif
	    actualName =  dir.path() + "/" + name + ".conf";
	}
    }

    return actualName;
}

/* This cannot be made public because of binary compat issues */
void Config::read( QTextStream &s )
{
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // The below should work, but doesn't in Qt 2.3.0
    s.setCodec( QTextCodec::codecForMib( 106 ) );
#else
    s.setEncoding( QTextStream::UnicodeUTF8 );
#endif

    QStringList list = QStringList::split('\n', s.read() );

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( !parse( *it ) ) {
            git = groups.end();
            return;
        }
    }
}


/*!
  \class Config config.h
  \brief The Config class provides for saving application configuration state.

  You should keep a Config in existence only while you do not want others
  to be able to change the state. There is no locking currently, but there
  may be in the future.

  Note that in Qtopia before 1.6, the 'const' forms of the read functions
  where not available.

  \ingroup qtopiaemb
*/

/*!
  \enum Config::Domain

  \value File
  \value User

  See \l{Config()} for details.
*/

/*!
  Constructs a configuration object that will load or create a
  configuration with the given \a name in the given \a domain.

  After construction, call setGroup() since almost every other
  function works in terms of the 'current group'.

  In the default Domain, \e User, the configuration is user-specific,
  and the \a name should not contain "/". This name should be globally
  unique.

  In the File Domain, \a name is an absolute filename.
*/
Config::Config( const QString &name, Domain domain )
    : filename( configFilename(name,domain) )
{
    git = groups.end();
    d = 0;
    read();
}

#ifdef QTOPIA_DESKTOP
Config::Config( QTextStream &s, Domain domain )
{
    Q_UNUSED( domain );
    git = groups.end();
    d = 0;
    read( s );
}
#endif

/*!
  Writes any changes to disk and destroys the in-memory object.
*/
Config::~Config()
{
    if ( changed )
	write();
    delete d;
}

/*!
  Returns TRUE if the current group has an entry called \a key;
  otherwise returns FALSE.
*/
bool Config::hasKey( const QString &key ) const
{
    if ( groups.end() == git )
	return FALSE;
    ConfigGroup::ConstIterator it = ( *git ).find( key );
    if ( it == ( *git ).end() ) {
	if ( d && !d->trcontext.isNull() ) {
	    it = ( *git ).find( key + "[]" );
	} else if ( d && d->multilang ) {
	    it = ( *git ).find( key + "["+lang+"]" );
	    if ( it == ( *git ).end() && !glang.isEmpty() )
		it = ( *git ).find( key + "["+glang+"]" );
	}
    }
    return it != ( *git ).end();
}

/*!
  Sets the current group for subsequent reading and writing of entries
  to \a gname. Grouping allows the application to partition the
  namespace.

  This function \e must be called prior to any reading or writing of
  entries.

  The \a gname must not be empty.

  \sa writeEntry() readEntry() readListEntry() readNumEntry() readBoolEntry()
*/
void Config::setGroup( const QString &gname )
{
    QMap< QString, ConfigGroup>::Iterator it = groups.find( gname );
    if ( it == groups.end() ) {
	git = groups.insert( gname, ConfigGroup() );
	changed = TRUE;
	return;
    }
    git = it;
}

/*!
  Writes a (\a key, \a value) entry to the current group.

  \sa readEntry()
*/
void Config::writeEntry( const QString &key, const char* value )
{
    writeEntry(key,QString(value));
}

/*!
  Writes a (\a key, \a value) entry to the current group.

  \sa readEntry()
*/
void Config::writeEntry( const QString &key, const QString &value )
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    if ( (*git)[key] != value ) {
	( *git ).insert( key, value );
	changed = TRUE;
    }
}

/*
  Note that the degree of protection offered by the encryption here is
  only sufficient to avoid the most casual observation of the configuration
  files. People with access to the files can write down the contents and
  decrypt it using this source code.

  Conceivably, and at some burden to the user, this encryption could
  be improved.
*/
static QString encipher(const QString& plain)
{
    // mainly, we make it long
    QString cipher;
    int mix=28730492;
    for (int i=0; i<(int)plain.length(); i++) {
	int u = plain[i].unicode();
	int c = u ^ mix;
	QString x = QString::number(c,36);
	cipher.append(QChar('a'+x.length()));
	cipher.append(x);
	mix *= u;
    }
    return cipher;
}

static QString decipher(const QString& cipher)
{
    QString plain;
    int mix=28730492;
    for (int i=0; i<(int)cipher.length();) {
	int l = cipher[i].unicode()-'a';
	QString x = cipher.mid(i+1,l); i+=l+1;
	int u = x.toInt(0,36) ^ mix;
	plain.append(QChar(u));
	mix *= u;
    }
    return plain;
}

/*!
  Writes a weakly encrypted (\a key, \a value) entry to the current group.

  Note that the degree of protection offered by the encryption is
  only sufficient to avoid the most casual observation of the configuration
  files.

  \sa readEntryCrypt()
*/
void Config::writeEntryCrypt( const QString &key, const QString &value )
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    QString evalue = encipher(value);
    if ( (*git)[key] != evalue ) {
	( *git ).insert( key, evalue );
	changed = TRUE;
    }
}

/*!
  Writes a (\a key, \a num) entry to the current group.

  \sa readNumEntry() readBoolEntry()
*/
void Config::writeEntry( const QString &key, int num )
{
    QString s;
    s.setNum( num );
    writeEntry( key, s );
}

#ifdef Q_HAS_BOOL_TYPE
/*!
  Writes a (\a key, \a b) entry to the current group. This is equivalent
  to writing a 0 or 1 as an integer entry.

  \sa readBoolEntry()
*/
void Config::writeEntry( const QString &key, bool b )
{
    QString s;
    s.setNum( ( int )b );
    writeEntry( key, s );
}
#endif

/*!
  Writes a (\a key, \a lst) entry to the current group. The list is
  separated by \a sep, so the strings must not contain that character.

  \sa readListEntry()
*/
void Config::writeEntry( const QString &key, const QStringList &lst, const QChar &sep )
{
    QString s;
    QStringList::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it )
	s += *it + sep;
    writeEntry( key, s );
}

/*!
  Removes the \a key entry from the current group. Does nothing if
  there is no such entry.

  \sa writeEntry() clearGroup()
*/

void Config::removeEntry( const QString &key )
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    ( *git ).remove( key );
    changed = TRUE;
}

/*!
  \fn bool Config::operator == ( const Config & other ) const

  Tests for equality with \a other. Config objects are equal if they
  refer to the same filename.

  \sa operator!=()
*/

/*!
  \fn bool Config::operator != ( const Config & other ) const

  Tests for inequality with \a other. Config objects are equal if they
  refer to the same filename.

  \sa operator==()
*/

/*!
  \fn QString Config::readEntry( const QString &key, const QString &deflt ) const

  Returns the string entry for \a key, defaulting to \a deflt if there
  is no entry for the given \a key.

  \sa writeEntry()
*/

/*!
  \internal
  For compatibility, non-const version.
*/
QString Config::readEntry( const QString &key, const QString &deflt )
{
    QString r;
    if ( d && !d->trcontext.isNull() ) {
	// Still try untranslated first, becuase:
	//  1. It's the common case
	//  2. That way the value can be WRITTEN (becoming untranslated)
	r = readEntryDirect( key );
	if ( !r.isNull() )
	    return r;
	r = readEntryDirect( key + "[]" );
	if ( !r.isNull() )
	    return LocalTranslator::translate(d->trfile,d->trcontext,r);
    } else if ( d && d->multilang ) {
	// For compatibilitity
	r = readEntryDirect( key + "["+lang+"]" );
	if ( !r.isNull() )
	    return r;
	if ( !glang.isEmpty() ) {
	    r = readEntryDirect( key + "["+glang+"]" );
	    if ( !r.isNull() )
		return r;
	}
    }
    r = readEntryDirect( key, deflt );
    return r;
}

/*!
  \fn QString Config::readEntryCrypt( const QString &key, const QString &deflt ) const

  Returns the unencrypted string entry for the encrypted entry stored
  using \a key, defaulting to \a deflt if there is no entry for the
  given \a key.

  \sa writeEntryCrypt()
*/

/*!
  \internal
  For compatibility, non-const version.
*/
QString Config::readEntryCrypt( const QString &key, const QString &deflt )
{
    QString res = readEntry( key );
    if ( res.isNull() )
	return deflt;
    return decipher(res);
}

/*!
  \fn QString Config::readEntryDirect( const QString &key, const QString &deflt ) const
  \internal
*/

/*!
  \internal
  For compatibility, non-const version.
*/
QString Config::readEntryDirect( const QString &key, const QString &deflt )
{
    if ( git == groups.end() ) {
	//qWarning( "no group set" );
	return deflt;
    }
    ConfigGroup::ConstIterator it = ( *git ).find( key );
    if ( it != ( *git ).end() )
	return *it;
    else
	return deflt;
}

/*!
  \fn int Config::readNumEntry( const QString &key, int deflt ) const

  Returns the integer entry stored using \a key, defaulting to \a
  deflt if there is no entry for the given \a key.

  \sa writeEntry()
*/

/*!
  \internal
  For compatibility, non-const version.
*/
int Config::readNumEntry( const QString &key, int deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return deflt;
    else
	return s.toInt();
}

/*!
  \fn bool Config::readBoolEntry( const QString &key, bool deflt ) const

  Returns the boolean entry stored (as an integer) using \a key,
  defaulting to \a deflt if there is no entry for the given \a key.

  \sa writeEntry()
*/

/*!
  \internal
  For compatibility, non-const version.
*/
bool Config::readBoolEntry( const QString &key, bool deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return deflt;
    else
	return (bool)s.toInt();
}

/*!
  \fn QStringList Config::readListEntry( const QString &key, const QChar &sep ) const

  Returns the string list entry stored using \a key and with \a sep as
  the separator.

  These entries are stored as a single string, with each element
  separated by \a sep.

  \sa writeEntry()
*/

/*!
  \internal
  For compatibility, non-const version.
*/
QStringList Config::readListEntry( const QString &key, const QChar &sep )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return QStringList();
    else
	return QStringList::split( sep, s );
}

/*!
  Removes all entries from the current group.

  \sa removeEntry() removeGroup()
*/
void Config::clearGroup()
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    if ( !(*git).isEmpty() ) {
	( *git ).clear();
	changed = TRUE;
    }
}

/*!
  \internal
*/
void Config::write( const QString &fn )
{
    QString strNewFile;
    if ( !fn.isEmpty() )
	filename = fn;
    strNewFile = qtopia_tempName( filename );
    QFile f( strNewFile );
    if ( !f.open( IO_WriteOnly|IO_Raw ) ) {
	qWarning( "could not open for writing `%s'", strNewFile.latin1() );
#ifndef QT_NO_COP
        //send this msg to make sure user is notified if it is
        //"out of disk space" problem
        QCopEnvelope e("QPE/System", "checkDiskSpace()");
#endif
	git = groups.end();
	return;
    }

    QString str;
    QCString cstr;
    QMap< QString, ConfigGroup >::Iterator g_it = groups.begin();

    for ( ; g_it != groups.end(); ++g_it ) {
 	str += "[" + g_it.key() + "]\n";
 	ConfigGroup::Iterator e_it = ( *g_it ).begin();
 	for ( ; e_it != ( *g_it ).end(); ++e_it )
 	    str += e_it.key() + " = " + *e_it + "\n";
    }
    cstr = str.utf8();

    int total_length;
    total_length = f.writeBlock( cstr.data(), cstr.length() );
    if ( total_length != int(cstr.length()) ) {
        QString msg = qApp->translate( "Config", "<qt>There was a problem creating "
				                "Configuration Information for this program."
						"<br>Please free up some space and try again.</qt>");
	qWarning( "unable to write configuration information" );
#ifndef QT_NO_COP
        QCopEnvelope e("QPE/System", "outOfDiskSpace(QString)");
        e << msg;
#endif
        f.close();
	QFile::remove( strNewFile );
	return;
    }

    f.close();
    qtopia_renameFile( strNewFile, filename );
#ifndef Q_OS_WIN32
    if (qpe_configCache)
	qpe_configCache->insert(filename, groups, d);
#endif
    changed = FALSE;
}

/*!
  Returns TRUE if the Config is in a valid state; otherwise returns
  FALSE.
*/
bool Config::isValid() const
{
    return groups.end() != git;
}

/*!
  \internal
*/
void Config::read()
{
    changed = FALSE;

    QString readFilename(filename);

    if ( !QFile::exists(filename) ) {
	bool failed = TRUE;
	QFileInfo fi(filename);
	QString settingsDir = QDir::homeDirPath() + "/Settings";
	if (fi.dirPath(TRUE) == settingsDir) {
	    // User setting - see if there is a default in $QPEDIR/etc/default/
	    QString dftlFile = QPEApplication::qpeDir() + "/etc/default/" + fi.fileName();
	    if (QFile::exists(dftlFile)) {
		readFilename = dftlFile;
		failed = FALSE;
	    }
	}
	if (failed) {
	    git = groups.end();
	    return;
	}
    }

#ifndef Q_OS_WIN32
    if (!qpe_configCache)
	qpe_configCache = new ConfigCache;

    if (qpe_configCache->find(readFilename, groups, d)) {
	if ( d && d->multilang ) {
	    QStringList l = Global::languageList();
	    lang = l[0];
	    glang = l[1];
	}
	git = groups.begin();
	return;
    }
#endif

    QFile f( readFilename );
    if ( !f.open( IO_ReadOnly ) ) {
	git = groups.end();
	return;
    }

    if (f.getch()!='[') {
	git = groups.end();
	return;
    }
    f.ungetch('[');

    QTextStream s( &f );
    read( s );
    f.close();

#ifndef Q_OS_WIN32
    qpe_configCache->insert(readFilename, groups, d);
#endif
}

/*!
  \internal
*/
bool Config::parse( const QString &l )
{
    QString line = l.stripWhiteSpace();
    if ( line[ 0 ] == QChar( '[' ) ) {
	QString gname = line;
	gname = gname.remove( 0, 1 );
	if ( gname[ (int)gname.length() - 1 ] == QChar( ']' ) )
	    gname = gname.remove( gname.length() - 1, 1 );
	git = groups.insert( gname, ConfigGroup() );
    } else if ( !line.isEmpty() ) {
	if ( git == groups.end() )
	    return FALSE;
	int eq = line.find( '=' );
	if ( eq == -1 )
	    return FALSE;
	QString key = line.left(eq).stripWhiteSpace();
	QString value = line.mid(eq+1).stripWhiteSpace();

	if ( git.key() == "Translation" ) {
	    if ( key == "File" ) {
		if ( !d )
		    d = new ConfigPrivate;
		d->trfile = value;
	    } else if ( key == "Context" ) {
		if ( !d )
		    d = new ConfigPrivate;
		d->trcontext = value.latin1();
            } else if ( key.startsWith("Comment") ) {
                return TRUE; // ignore comment for ts file
	    } else {
		return FALSE; // Unrecognized
	    }
	}

	int kl = key.length();
	if ( kl > 1 && key[kl-1] == ']' && key[kl-2] != '[' ) {
	    // Old-style translation (inefficient)
	    if ( !d )
		d = new ConfigPrivate;
	    if ( !d->multilang ) {
	        QStringList l = Global::languageList();
		lang = l[0];
		glang = l[1];
		d->multilang = TRUE;
	    }
	}

	( *git ).insert( key, value );
    }
    return TRUE;
}
