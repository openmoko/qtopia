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
#include "filemanager.h"
#include "applnk.h"
#include "global.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <errno.h>
#include <stdlib.h>

// defined in applnk.cpp
bool mkdirRecursive( QString path );

/*!
  \class FileManager
  \brief The FileManager class assists with AppLnk input/output.

  The FileManager class is used to \l{loadFile()}s and
  \l{saveFile()}s. It also provides a copyFile() function and a
  function to test for a file's existence, exists(). It also provides
  openFile() that provides an IO device for reading.

  \ingroup qtopiaemb
*/

/*!
  Constructs a FileManager object.
*/
FileManager::FileManager()
{
}

/*!
  Destroys the FileManager object.
*/
FileManager::~FileManager()
{

}

/*!
  \overload
  Saves \a data as the document specified by \a f.

  Returns TRUE if the operation succeeded; otherwise returns FALSE.
*/
bool FileManager::saveFile( const DocLnk &f, const QByteArray &data )
{
    QString fn = qtopia_tempName( f.file() );
    ensurePathExists( fn );
    QFile fl( fn );
    if ( !fl.open( IO_WriteOnly|IO_Raw ) )
	return FALSE;
    int total_written = fl.writeBlock( data );
    fl.close();
    if ( total_written != int(data.size()) || !f.writeLink() ) {
	QFile::remove( fn );
	return FALSE;
    }
    qtopia_renameFile( fn, f.file() );
    return TRUE;
}

/*!
  Saves \a text as the document specified by \a f.

  The text is saved in UTF8 format.

  Returns TRUE if the operation succeeded; otherwise returns FALSE.
*/
bool FileManager::saveFile( const DocLnk &f, const QString &text )
{
    QString fn = qtopia_tempName( f.file() );
    ensurePathExists( fn );
    QFile fl( fn );
    if ( !fl.open( IO_WriteOnly|IO_Raw ) ) {
	qDebug( "open failed: %s", fn.latin1() );
	return FALSE;
    }

    QCString cstr = text.utf8();
    int total_written;
    total_written = fl.writeBlock( cstr.data(), cstr.length() );
    fl.close();
    if ( total_written != int(cstr.length()) || !f.writeLink() ) {
	QFile::remove( fn );
	return FALSE;
    }
    // okay now rename the file...
    qtopia_renameFile( fn, f.file() );
    return TRUE;
}


/*!
  \overload
  Loads \a text from the document specified by \a f.

  The text must be in UTF8 format.

  Returns TRUE if the operation succeeded; otherwise returns FALSE.
*/
bool FileManager::loadFile( const DocLnk &f, QString &text )
{
    QString fn = f.file();
    QFile fl( fn );
    if ( !fl.open( IO_ReadOnly ) )
	return FALSE;
    QTextStream ts( &fl );
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // The below should work, but doesn't in Qt 2.3.0
    ts.setCodec( QTextCodec::codecForMib( 106 ) );
#else
    ts.setEncoding( QTextStream::UnicodeUTF8 );
#endif
    text = ts.read();
    fl.close();
    return TRUE;
}


/*!
  Loads \a ba from the document specified by \a f.

  Returns TRUE if the operation succeeded; otherwise returns FALSE.
*/
bool FileManager::loadFile( const DocLnk &f, QByteArray &ba )
{
    QString fn = f.file();
    QFile fl( fn );
    if ( !fl.open( IO_ReadOnly ) )
	return FALSE;
    ba.resize( fl.size() );
    if ( fl.size() > 0 )
	fl.readBlock( ba.data(), fl.size() );
    fl.close();
    return TRUE;
}

/*!
  Copies the document specified by \a src to the document specified
  by \a dest.

  Returns TRUE if the operation succeeded; otherwise returns FALSE.
*/
bool FileManager::copyFile( const AppLnk &src, const AppLnk &dest )
{
    QFile sf( src.file() );
    if ( !sf.open( IO_ReadOnly ) )
	return FALSE;

    QString fn = qtopia_tempName( dest.file() );
    ensurePathExists( fn );
    QFile df( fn );
    if ( !df.open( IO_WriteOnly|IO_Raw ) )
	return FALSE;

    const int bufsize = 16384;
    char buffer[bufsize];
    bool ok = TRUE;
    int bytesRead = 0;
    while ( ok && !sf.atEnd() ) {
	bytesRead = sf.readBlock( buffer, bufsize );
	if ( bytesRead < 0 )
	    ok = FALSE;
	while ( ok && bytesRead > 0 ) {
	    int bytesWritten = df.writeBlock( buffer, bytesRead );
	    if ( bytesWritten < 0 )
		ok = FALSE;
	    else
		bytesRead -= bytesWritten;
	}
    }

    // If file backing the QFile df goes away (or runs out of room) then
    // the error wont be picked up until the process file buffer is flushed
    // ie typically on close.

    // Does not return status, must do explicit check
    df.close();
    ok = ( df.status() == IO_Ok );

    if ( df.exists() )
    {
        // df is suspect corrupted, and source not yet removed
        // remove now spurious tmp file
        if ( ! ok ) df.remove();
    }
    else // if target filesystem was removed for example
    {
        ok = false;
    }

    // all content was successfully read?
    sf.close();
    ok = ( sf.status() == IO_Ok );

    if ( ok )
	ok = dest.writeLink();

    if ( ok )
	qtopia_renameFile( fn, dest.file() );

    return ok;
}

/*!
  Opens the document specified by \a f as a readable QIODevice.
  The caller must delete the return value.

  Returns 0 if the operation fails.
*/
QIODevice* FileManager::openFile( const DocLnk& f )
{
    QString fn = f.file();
    QFile* fl = new QFile( fn );
    if ( !fl->open( IO_ReadOnly ) ) {
	delete fl;
        fl = 0;
    }
    return fl;
}

/*!
  \overload
  Opens the document specified by \a f as a writable QIODevice.
  The caller must delete the return value.

  Returns 0 if the operation fails.
*/
QIODevice* FileManager::saveFile( const DocLnk& f )
{
    QString fn = f.file();
    ensurePathExists( fn );
    QFile* fl = new QFile( fn );
    if ( fl->open( IO_WriteOnly ) ) {
	f.writeLink();
    } else {
	delete fl;
        fl = 0;
    }
    return fl;
}

/*!
  Returns TRUE if the document specified by \a f current exists
  as a file on disk; otherwise returns FALSE.
*/
bool FileManager::exists( const DocLnk &f )
{
    return QFile::exists(f.file());
}


/*!
  Ensures that the path \a fn exists, by creating required directories.

  Returns TRUE if it succeededs; otherwise returns FALSE.
*/
bool FileManager::ensurePathExists( const QString &fn )
{
    QFileInfo fi(fn);
    if (!fi.isDir()){
      fi = QFileInfo(fi.dirPath(TRUE));
    }

    QString dirPath = fi.absFilePath();
    if ( !fi.exists() ) {
#ifndef Q_OS_WIN32
	// May need to create directories
	QString cmdLine(QString("mkdir -p ") + dirPath.latin1());
	if ( system(cmdLine.latin1())){
#else
	if ( !mkdirRecursive( dirPath ) ) {
#endif
	     qDebug("FileManager::ensurePathExists : System failed to create directory path %s", dirPath.latin1());
	     return FALSE;
	}
    }
    return TRUE;
}
