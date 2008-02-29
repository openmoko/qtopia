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

#include "qplatformdefs.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qt_mac.h"

#if defined(Q_OS_MACX)
#include <pwd.h>
#include <grp.h>
#endif
#include <limits.h>

void QFileInfo::slashify( QString& n )
{
    for ( int i = 0; i < (int)n.length(); i++ ) {
	if( n[i] == ':' )
	    n[i] = '/';
    }
}

void QFileInfo::makeAbs( QString & )
{
    return;
}

bool QFileInfo::isFile() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & S_IFMT) == S_IFREG : FALSE;
}

bool QFileInfo::isDir() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & S_IFMT) == S_IFDIR : FALSE;
}

bool QFileInfo::isSymLink() const
{
#if defined(Q_OS_UNIX)
    if ( !fic || !cache )
	doStat();
    return symLink;
#else
    return FALSE;
#endif
}

QString QFileInfo::readLink() const
{
#if defined(Q_OS_MACX)
    char s[PATH_MAX+1];
    if ( !isSymLink() )
	return QString();
    int len = readlink( QFile::encodeName(QDir::convertSeparators(fn)).data(), s, PATH_MAX );
    if ( len >= 0 ) {
	s[len] = '\0';
	return QFile::decodeName(s);
    }
#endif
    return QString();
}

static const uint nobodyID = (uint) -2;

QString QFileInfo::owner() const
{
#if defined(Q_OS_MACX)
    passwd *pw = getpwuid( ownerId() );
    if ( pw )
	return QFile::decodeName( pw->pw_name );
#endif
    return QString::null;
}

uint QFileInfo::ownerId() const
{
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return fic->st.st_uid;
    return nobodyID;
}

QString QFileInfo::group() const
{
#if defined(Q_OS_MACX)
    struct group *gr = getgrgid( groupId() );
    if ( gr )
	return QFile::decodeName( gr->gr_name );
#endif
    return QString::null;
}

uint QFileInfo::groupId() const
{
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return fic->st.st_gid;
    return nobodyID;
}

bool QFileInfo::permission( int permissionSpec ) const
{
    if ( !fic || !cache )
	doStat();
    if ( fic ) {
	uint mask = 0;
	if ( permissionSpec & ReadOwner )
	    mask |= S_IRUSR;
	if ( permissionSpec & WriteOwner )
	    mask |= S_IWUSR;
	if ( permissionSpec & ExeOwner )
	    mask |= S_IXUSR;
	if ( permissionSpec & ReadUser )
	    mask |= S_IRUSR;
	if ( permissionSpec & WriteUser )
	    mask |= S_IWUSR;
	if ( permissionSpec & ExeUser )
	    mask |= S_IXUSR;
	if ( permissionSpec & ReadGroup )
	    mask |= S_IRGRP;
	if ( permissionSpec & WriteGroup )
	    mask |= S_IWGRP;
	if ( permissionSpec & ExeGroup )
	    mask |= S_IXGRP;
	if ( permissionSpec & ReadOther )
	    mask |= S_IROTH;
	if ( permissionSpec & WriteOther )
	    mask |= S_IWOTH;
	if ( permissionSpec & ExeOther )
	    mask |= S_IXOTH;
	if ( mask ) {
	   return (fic->st.st_mode & mask) == mask;
	} else {
#if defined(QT_CHECK_NULL)
	   qWarning( "QFileInfo::permission: permissionSpec is 0" );
#endif
	   return TRUE;
	}
    } else {
	return FALSE;
    }
}

void QFileInfo::doStat() const
{
    QFileInfo *that = ((QFileInfo*)this);	// mutable function
    if ( !that->fic )
	that->fic = new QFileInfoCache;
#if defined(Q_OS_UNIX)
    that->symLink = FALSE;
#endif
    struct stat *b = &that->fic->st;
    int r = ::stat( QFile::encodeName(QDir::convertSeparators(fn)), b );
    if ( r != 0 ) {
	delete that->fic;
	that->fic = 0;
    }
}

#ifndef QT_NO_DIR
QString QFileInfo::dirPath( bool absPath ) const
{
    QString s;
    if ( absPath )
	s = absFilePath();
    else
	s = fn;
    int pos = s.findRev( '/' );
    if ( pos == -1 ) {
	return QString::fromLatin1( "." );
    } else {
	if ( pos == 0 )
	    return QString::fromLatin1( "/" );
	return s.left( pos );
    }
}
#endif

QString QFileInfo::fileName() const
{
    int p = fn.findRev( '/' );
    if ( p == -1 ) {
	return fn;
    } else {
	return fn.mid( p + 1 );
    }
}
