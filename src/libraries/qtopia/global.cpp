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


#define QTOPIA_INTERNAL_LANGLIST
#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif
#include <qtopia/global.h>

#ifdef Q_WS_QWS
#include <qtopia/qpedebug.h>
#include <qtopia/qdawg.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/storage.h>
#include <qtopia/applnk.h>
#endif
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qfile.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qmap.h>
#include <qdict.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qdatetime.h>

#ifdef QTOPIA_DESKTOP
#include <qsettings.h>
#endif

#ifndef Q_OS_WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <qfileinfo.h>
#else
#include <sys/locking.h>
#include <io.h>
#include <string.h> // for strerror function
#include <stdlib.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

extern bool mkdirRecursive( QString path );

// added in qtopia 1.6, so don't export this
QString qtopia_tempName( const QString &fname )
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

bool qtopia_renameFile( QString from, QString to )
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

#ifdef Q_WS_QWS

Global::Command* Global::builtin=0;
QGuardedPtr<QWidget> *Global::running=0;

//#include "quickexec_p.h"
class Emitter : public QObject {
    Q_OBJECT
public:
    Emitter( QWidget* receiver, const QString& document )
    {
	connect(this, SIGNAL(setDocument(const QString&)),
	    receiver, SLOT(setDocument(const QString&)));
	emit setDocument(document);
	disconnect(this, SIGNAL(setDocument(const QString&)),
	       receiver, SLOT(setDocument(const QString&)));
    }

signals:
    void setDocument(const QString&);
};


static bool docDirCreated = FALSE;
static QDawg* fixed_dawg = 0;
static QDict<QDawg> *named_dawg = 0;

static QString dictDir()
{
    return QPEApplication::qpeDir() + "etc/dict";
}

/*!
  \class Global global.h
  \brief The Global class provides application-wide global functions.

  The Global functions are grouped as follows:
  \tableofcontents

  \section1 User Interface

  The statusMessage() function provides short-duration messages to the
  user. The showInputMethod() function shows the current input method,
  and hideInputMethod() hides the input method.

  \section1 Document related

  The findDocuments() function creates a set of \link doclnk.html
  DocLnk\endlink objects in a particular folder.

  \section1 Filesystem related

  Global provides an applicationFileName() function that returns the
  full path of an application-specific file.

  The execute() function runs an application.

  \section1 Word list related

  A list of words relevant to the current locale is maintained by the
  system. The list is held in a \link qdawg.html DAWG\endlink
  (implemented by the QDawg class). This list is used, for example, by
  the pickboard input method.

  The global QDawg is returned by fixedDawg(); this cannot be updated.
  An updatable copy of the global QDawg is returned by addedDawg().
  Applications may have their own word lists stored in \l{QDawg}s
  which are returned by dawg(). Use addWords() to add words to the
  updateable copy of the global QDawg or to named application
  \l{QDawg}s.

  \section1 Quoting

  The shellQuote() function quotes a string suitable for passing to a
  shell. The stringQuote() function backslash escapes '\' and '"'
  characters.

  \section1 Hardware

  The writeHWClock() function sets the hardware clock to the system
  clock's date and time.

  \ingroup qtopiaemb
*/

/*!
  \internal
*/
Global::Global()
{
}

/*!
  Returns the unchangeable QDawg that contains general
  words for the current locale.

  \sa addedDawg()
*/
const QDawg& Global::fixedDawg()
{
    if ( !fixed_dawg ) {
	if ( !docDirCreated )
	    createDocDir();

	fixed_dawg = new QDawg;
	QString dawgfilename = dictDir() + "/dawg";
	QString words_lang;
	QStringList langs = Global::languageList();
	for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
	    QString lang = *it;
	    words_lang = dictDir() + "/words." + lang;
	    QString dawgfilename_lang = dawgfilename + "." + lang;
	    if ( QFile::exists(dawgfilename_lang) ||
		 QFile::exists(words_lang) ) {
		dawgfilename = dawgfilename_lang;
		break;
	    }
	}
	QFile dawgfile(dawgfilename);

	if ( !dawgfile.exists() ) {
	    QString fn = dictDir() + "/words";
	    if ( QFile::exists(words_lang) )
		fn = words_lang;
	    QFile in(fn);
	    if ( in.open(IO_ReadOnly) ) {
		fixed_dawg->createFromWords(&in);
		dawgfile.open(IO_WriteOnly);
		fixed_dawg->write(&dawgfile);
		dawgfile.close();
	    }
	} else {
	    fixed_dawg->readFile(dawgfilename);
	}
    }

    return *fixed_dawg;
}

/*!
  Returns the changeable QDawg that contains general
  words for the current locale.

  \sa fixedDawg()
*/
const QDawg& Global::addedDawg()
{
    return dawg("local"); // No tr
}

/*!
  Returns the QDawg with the given \a name.
  This is an application-specific word list.

  \a name should not contain "/".
*/
const QDawg& Global::dawg(const QString& name)
{
    createDocDir();
    if ( !named_dawg )
	named_dawg = new QDict<QDawg>;
    QDawg* r = named_dawg->find(name);
    if ( !r ) {
	r = new QDawg;
	named_dawg->insert(name,r);
	QString dawgfilename = applicationFileName("Dictionary", name) + ".dawg"; // No tr
	QFile dawgfile(dawgfilename);
	if ( dawgfile.open(IO_ReadOnly) )
	    r->readFile(dawgfilename);
    }
    return *r;
}

/*!
  \overload
  Adds \a wordlist to the addedDawg().

  Note that the addition of words persists between program executions
  (they are saved in the dictionary files), so you should confirm the
  words with the user before adding them.
*/
void Global::addWords(const QStringList& wordlist)
{
    addWords("local",wordlist); // No tr
}

/*!
  Adds \a wordlist to the dawg() named \a dictname.

  Note that the addition of words persists between program executions
  (they are saved in the dictionary files), so you should confirm the
  words with the user before adding them.
*/
void Global::addWords(const QString& dictname, const QStringList& wordlist)
{
    QDawg& d = (QDawg&)dawg(dictname);
    QStringList all = d.allWords() + wordlist;
    d.createFromWords(all);

    QString dawgfilename = applicationFileName("Dictionary", dictname) + ".dawg"; // No tr
    QString dawgfilenamenew = dawgfilename + ".new";
    QFile dawgfile(dawgfilenamenew);
    if ( dawgfile.open(IO_WriteOnly) ) {
	d.write(&dawgfile);
	dawgfile.close();
	qtopia_renameFile(dawgfilenamenew,dawgfilename);
    }

    // #### Re-read the dawg here if we use mmap().

    // #### Signal other processes to re-read.
}


/*!
  \internal
*/
void Global::createDocDir()
{
    if ( !docDirCreated ) {
	QDir d;
	if (!d.exists(QPEApplication::documentDir().latin1())){
	    docDirCreated = TRUE;
#ifndef Q_WS_WIN32
	    mkdir( QPEApplication::documentDir().latin1(), 0755 );
#else
	    mkdirRecursive(QPEApplication::documentDir());
#endif
	}else{
	    docDirCreated = TRUE;
	}
    }
}


/*!
  \internal
*/
void Global::applyStyle()
{
#ifndef QT_NO_COP
    QCopChannel::send( "QPE/System", "applyStyle()" );
#else
    ((QPEApplication *)qApp)->applyStyle(); // apply without needing QCop for floppy version
#endif
}

/*!
  \internal
*/
QWidget *Global::shutdown( bool )
{
#ifndef QT_NO_COP
    QCopChannel::send( "QPE/System", "shutdown()" );
#endif
    return 0;
}

/*!
  \internal
*/
QWidget *Global::restart( bool )
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel::send( "QPE/System", "restart()" );
#endif
    return 0;
}

/*!
  Explicitly show the current input method.

  Input methods are indicated in the taskbar by a small icon. If the
  input method is activated (shown) then it takes up some proportion
  of the bottom of the screen, to allow the user to interact (input
  characters) with it.

  \sa hideInputMethod()
*/
void Global::showInputMethod()
{
#ifndef QT_NO_COP
    QCopChannel::send( "QPE/TaskBar", "showInputMethod()" );
#endif
}

/*!
  Explicitly hide the current input method.

  The current input method is still indicated in the taskbar, but no
  longer takes up screen space, and can no longer be interacted with.

  \sa showInputMethod()
*/
void Global::hideInputMethod()
{
#ifndef QT_NO_COP
    QCopChannel::send( "QPE/TaskBar", "hideInputMethod()" );
#endif
}


/*!
  \internal
*/
bool Global::isBuiltinCommand( const QString &name )
{
    if(!builtin)
	return FALSE; // yes, it can happen
    for (int i = 0; builtin[i].file; i++) {
	if ( builtin[i].file == name ) {
	    return TRUE;
	}
    }

    return FALSE;
}

/*!
  \class Global::Command
  \brief The Global::Command class is internal.
  \internal
*/

/*!
  \internal
*/
void Global::setBuiltinCommands( Command* list )
{
    if ( running )
	delete [] running;

    builtin = list;
    int count = 0;
    if (!builtin)
	return;
    while ( builtin[count].file )
	count++;

    running = new QGuardedPtr<QWidget> [ count ];
}

/*!
  \internal
*/
void Global::setDocument( QWidget* receiver, const QString& document )
{
    Emitter emitter(receiver,document);
}

/*!
  \internal
*/
bool Global::terminateBuiltin( const QString& n )
{
    if (!builtin)
	return FALSE;
    for (int i = 0; builtin[i].file; i++) {
	if ( builtin[i].file == n ) {
	    delete running[i];
	    return TRUE;
	}
    }

    return FALSE;
}

/*!
  \internal
*/
void Global::terminate( const AppLnk* app )
{
    //if ( terminateBuiltin(app->exec()) ) return; // maybe? haven't tried this

#ifndef QT_NO_COP
    QCString channel = "QPE/Application/" + app->exec().utf8();
    if ( QCopChannel::isRegistered(channel) ) {
	QCopEnvelope e(channel, "quit()");
    }
#endif
}


/*!
  Low-level function to run command.

  \warning Do not use this function. Use execute instead.

  \sa execute()
*/
void Global::invoke(const QString &)
{
    qDebug("Global::invoke does not work anymore");
}

/*!
  Executes the application identfied by \a c, passing \a
  document if it isn't null.

  Note that a better approach might be to send a QCop message to the
  application's QPE/Application/\e{appname} channel.
*/
void Global::execute( const QString &c, const QString& document )
{
#ifndef QT_NO_COP
    if ( document.isNull() ) {
	QCopEnvelope e( "QPE/System", "execute(QString)" );
	e << c;
    } else {
	QCopEnvelope e( "QPE/System", "execute(QString,QString)" );
	e << c << document;
    }
#endif
    return;

}

#endif

/*!
  Returns the string \a s with the characters '\', '"', and '$' quoted
  by a preceeding '\', and enclosed by double-quotes (").

  \sa stringQuote()
*/
QString Global::shellQuote(const QString& s)
{
    QString r="\"";
    for (int i=0; i<(int)s.length(); i++) {
	char c = s[i].latin1();
	switch (c) {
	    case '\\': case '"': case '$':
		r+="\\";
	}
	r += s[i];
    }
    r += "\"";
    return r;
}

/*!
  Returns the string \a s with the characters '\' and '"' quoted by a
  preceeding '\'.

  \sa shellQuote()
*/
QString Global::stringQuote(const QString& s)
{
    QString r="\"";
    for (int i=0; i<(int)s.length(); i++) {
	char c = s[i].latin1();
	switch (c) {
	    case '\\': case '"':
		r+="\\";
	}
	r += s[i];
    }
    r += "\"";
    return r;
}

#ifdef Q_WS_QWS

/*!
  Finds all documents in the system's document directories which
  match the filter \a mimefilter, and appends the resulting \link
  doclnk.html DocLnk\endlink objects to \a folder.
*/
void Global::findDocuments(DocLnkSet* folder, const QString &mimefilter)
{
    QString homedocs = QDir::homeDirPath() + "/Documents";
    DocLnkSet d(homedocs,mimefilter);
    folder->appendFrom(d);
    StorageInfo storage;
    const QList<FileSystem> &fs = storage.fileSystems();
    QListIterator<FileSystem> it ( fs );
    for ( ; it.current(); ++it ) {
	if ( (*it)->isRemovable() ) {
	    QString path = (*it)->path();
	    DocLnkSet ide( path, mimefilter );
	    folder->appendFrom(ide);
	}
    }
}
#endif // Q_WS_QWS

QStringList Global::languageList()
{
    QString lang;
#ifdef QTOPIA_DESKTOP
    QSettings settings;
    settings.insertSearchPath( QSettings::Unix,
	    QDir::homeDirPath() + "/.palmtopcenter/" );
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    lang = settings.readEntry( "/palmtopcenter/language" );
#endif
    if (lang.isEmpty())
	lang = getenv("LANG");

    QStringList langs;
    langs.append(lang);
    int i  = lang.find(".");
    if ( i > 0 )
	lang = lang.left( i );
    i = lang.find( "_" );
    if ( i > 0 )
	langs.append(lang.left(i));
    return langs;
}

#ifdef Q_WS_QWS
QStringList Global::helpPath()
{
    QStringList path;
    QStringList langs = Global::languageList();
    for (QStringList::ConstIterator it = langs.fromLast(); it!=langs.end(); --it) {
	QString lang = *it;
	if ( !lang.isEmpty() )
	    path += QPEApplication::qpeDir() + "help/" + lang + "/html";
    }
    path += QPEApplication::qpeDir() + "pics";
    path += QPEApplication::qpeDir() + "help/html";
    path += QPEApplication::qpeDir() + "docs";
    return path;
}
#endif

/*!
  Returns the full path for the application called \a appname, with the
  given \a filename. Returns QString::null if there was a problem creating
  the directory tree for \a appname.
  If \a filename contains "/", it is the caller's responsibility to
  ensure that those directories exist.
*/
QString Global::applicationFileName(const QString& appname, const QString& filename)
{
    QDir d;
    QString r = QDir::homeDirPath();
#ifdef QTOPIA_DESKTOP
    r += "/.palmtopcenter/";
#else
    r += "/Applications/";
#endif
    if ( !QFile::exists( r ) )
	if ( d.mkdir(r) == false )
	    return QString::null;
    r += appname;
    if ( !QFile::exists( r ) )
	if ( d.mkdir(r) == false )
	    return QString::null;
    r += "/"; r += filename;

    //qDebug("Global::applicationFileName = %s", r.latin1());
    return r;
}

/*!
  Displays a status \a message to the user. This usually appears
  in the taskbar for a short amount of time, then disappears.
*/
void Global::statusMessage(const QString& message)
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopEnvelope e( "QPE/TaskBar", "message(QString)" );
    e << message;
#endif
}

#ifdef Q_WS_QWS
#ifdef QTOPIA_INTERNAL_FILEOPERATIONS
  /*! \enum Global::Lockflags
    \internal
     This enum controls what type of locking is performed on file.

     Current defined values are:

     \value LockShare Allow lock to be shared. Reserved for future use
     \value LockWrite Create at a write lock.
     \value LockBlock Block the process when lock is encountered. Under WIN32
                      this blocking is limited to ten(10) failed attempts to
                      access locked file. Reserved for future use.

   */
#endif


#ifndef Q_OS_WIN32
/*!
  \internal
  Lock region of file. Any locks created should be released before the program exits. Returns TRUE if sucessfull

  \a f must be an open file
  \a flags any combination of LockShare, LockWrite, LockBlock
 */
bool Global::lockFile(QFile &f, int flags){
  struct flock fileLock;

  if (!f.isOpen())
      return FALSE;

  fileLock.l_whence = SEEK_SET;
  fileLock.l_start = 0;
  int lockCommand, lockMode;

  fileLock.l_len = f.size();


  if (flags == -1){
      lockMode =  F_RDLCK;
      if (f.mode() == IO_ReadOnly)
	  fileLock.l_type = F_RDLCK;
      else
	  fileLock.l_type = F_WRLCK;
      lockCommand = F_SETLK;
  }else{
      if (flags & Global::LockWrite)
	  fileLock.l_type = F_WRLCK;
      else
	  fileLock.l_type = F_RDLCK;
      if (flags & Global::LockBlock)
	  lockCommand = F_SETLK;
      else
	  lockCommand = F_SETLKW; // block process if possible
  }

  if (::fcntl(f.handle(), lockCommand, &fileLock) != -1)
      return TRUE;
  else
      return FALSE;
}


/*!
  \internal
  Unlock a region of file
  \a f must be an open file previously locked
 */
bool Global::unlockFile(QFile &f)
{
  struct flock fileLock;

  if (!f.isOpen())
      return FALSE;

  fileLock.l_whence = SEEK_SET;
  fileLock.l_start = 0;

  fileLock.l_len = f.size();

  fileLock.l_type = F_UNLCK;

  if (::fcntl(f.handle(), F_SETLK, &fileLock) != -1)
      return TRUE;
  else
      return FALSE;

}

/*!
  \internal
 Could a request to lock file with given flags succeed
 \a f must be an opened file
 \a flags the desired lock type required
*/
bool Global::isFileLocked(QFile &f, int /* flags */)
{
  struct flock fileLock;

  if (!f.isOpen())
      return FALSE;

  fileLock.l_whence = SEEK_SET;
  fileLock.l_start = 0;

  fileLock.l_len = f.size();

  if (f.mode() == IO_ReadOnly)
      fileLock.l_type = F_RDLCK;
  else
      fileLock.l_type = F_WRLCK;

  fileLock.l_pid = 0;

  if (::fcntl(f.handle(), F_SETLK, &fileLock) != -1)
      return FALSE;

  return fileLock.l_pid != 0;
}

/*!
  \internal
  Truncate file to size specified
  \a f must be an open file
  \a size must be a positive value
 */
bool Global::truncateFile(QFile &f, int size){
    if (!f.isOpen())
      return FALSE;

    return ::ftruncate(f.handle(), size) != -1;
}

#else

bool Global::lockFile(QFile &f, int flags)
{
    // If the file has been opened then a lock has been achieved
    return f.isOpen();
}

bool Global::unlockFile(QFile &f)
{
    // No need to do anything as we do not open file using sharing
    return TRUE;
}

bool Global::truncateFile(QFile &f, int size)
{
    if (!f.isOpen())
      return FALSE;

    if (size == -1)
	size = f.size();

    if (::chsize(f.handle(), size) != -1)
	return TRUE;
    else
	return FALSE;
}

bool Global::isFileLocked(QFile &f, int flags)
{
    // if the file is open then we must have achieved a file lock
    return f.isOpen();
}

#endif

#include "global.moc"

#endif  //qws


