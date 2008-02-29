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

#include <qtopia/applnk.h>

#include <qtopia/mimetype.h>
#include <qtopia/filemanager.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <stdlib.h>

#ifdef Q_OS_WIN32
/*
    Takes an absolute path which needs to exist.
    Creates all the intermediate paths up to that path as required.
    UNIX path seperators assumed, but will work for absolute Windows
    paths ie which start with "C:"
    ### Probably needs optimizing
 */
static bool mkdirRecursive( QString path )
{
    if ( path[int( path.length()) ] == '/' )
	path = path.left( path.length() - 1 );

    QString wholePath = path;

    QFileInfo fi( path );

    // find the path segment that does exist
    while ( !fi.isDir() && path.contains( '/' ) ) {
	path = path.left( path.findRev( '/' ) );
	fi = QFileInfo( path );
    }

    // make the path segments that do not exist
    QDir d;
    while ( path != wholePath ) {
	path = wholePath.left( wholePath.find( '/', path.length() + 1 ) );
	d.mkdir( path );
	fi = QFileInfo( path );
	if ( !fi.isDir() )
	    return FALSE;
    }

    return TRUE;
}
#endif

class AppLnkPrivate
{
public:
    // CUT DOWN COPY FROM applnk.cpp
    QStringList mCatList;
    QArray<int> mCat;
};

bool AppLnk::fileKnown() const
{
    return !mFile.isNull();
}

bool AppLnk::linkFileKnown() const
{
    return !mLinkFile.isNull();
}

QString AppLnk::icon() const
{
    if (mIconFile.isEmpty())
	return mIconFile;
    return mExec + '/' + mIconFile;
}

static bool prepareDirectories(const QString& lf)
{
    QFileInfo fi(lf);
    fi.setFile( fi.dirPath(TRUE) );

    QString dirPath = fi.absFilePath();
    if ( !fi.exists() ) {
#ifndef Q_OS_WIN32
	// May need to create directories
	QString cmdLine("mkdir -p ");
	cmdLine += dirPath.latin1();
	if ( system(cmdLine.latin1())){
#else
	if ( !mkdirRecursive( dirPath ) ) {
#endif
	     qDebug("AppLnk1::prepareDirectories System failed to create directory %s", dirPath.latin1());
	     return FALSE;
	}
    }
    return TRUE;
}

/*!
  This function sets the location for an AppLnk to \a docPath.

  First availability: Qtopia 1.6
*/

bool AppLnk::setLocation( const QString& docPath )
{
    //create it in default storage, if it doesn't exist.
    
    QString fileName = file();
    QString linkfileName = linkFile();
    
    AppLnk oldLnk = *this;
    
    //find unique names in the new location
    
    QFileInfo fi( fileName );
    
    QString baseName = docPath;
    if( baseName.right(1) != "/" )
        baseName += "/";
    baseName += type();
    if( baseName.right(1) != "/" )
        baseName += "/";
    baseName += fi.baseName();
    QString ext = fi.extension();
    if ( !ext.isEmpty() )
	ext = "." + ext;
    
    // Search for a unique name.
    if ( QFile::exists( baseName + ext ) ||
	 QFile::exists( baseName + ".desktop" ) ) {

	// It's already in the right place.
	if ( baseName + ext == fileName
	    && baseName + ".desktop" == linkfileName )
	    return TRUE;

	int n;
	QString nn;
	for(n = 1; n < 100; ++n)
	{
	    nn = baseName + "_" + QString::number(n);
	    if ( !QFile::exists( nn + ext ) &&
		    !QFile::exists( nn + ".desktop" ) ) {
		baseName = nn;
		break;
	    }
	}
	/// the  n_99 thing didn't work. try simplifying basename
	// to 5 alpha chars + num.
	if ( QFile::exists( baseName + ext ) ||
		QFile::exists( baseName + ".desktop" ) ) {
	    QString simpleBaseName = fi.baseName();
	    uint sindex = 0;

	    // inefficent, but good enough.
	    // basically only letters, 5 long, (since we add another possible 3 later.
	    // and not going to turn into utf8.  basically for dumb file systems.
	    while(sindex < simpleBaseName.length() && sindex < 6)
		if (!(simpleBaseName[(int)sindex].isLetter() && simpleBaseName[(int)sindex].latin1()))
		    simpleBaseName = simpleBaseName.left(sindex) + simpleBaseName.mid(sindex+1);
		else 
		    ++sindex;
	    if (simpleBaseName.length() < 1)
		simpleBaseName = "qfile"; // No tr
	    if (simpleBaseName.length() >= 6)
		simpleBaseName = simpleBaseName.left(5);

	    baseName = docPath + "/" + type() + "/" + simpleBaseName;
	    
	    for(n = 1; n < 100; ++n)
	    {
		nn = baseName + "_" + QString::number(n);
		if ( !QFile::exists( nn + ext ) &&
			!QFile::exists( nn + ".desktop" ) ) {
		    baseName = nn;
		    break;
		}
	    }
	}

	if ( QFile::exists( baseName + ext ) ||
		QFile::exists( baseName + ".desktop" ) )
	    return FALSE;
    }

    // Set the file and link file within the document object.
    setFile( baseName + ext );
    setLinkFile( baseName + ".desktop" );


    //move the files to the new location
    prepareDirectories(mFile);
    
    FileManager fm;
    bool success = fm.copyFile( oldLnk, *this );
    if ( success ) { 
	oldLnk.removeFiles();
    }
    return success;
}

/*!
  Returns an AppLnk that is a deep copy of \a copy.

  First availability: Qtopia 1.6
*/

AppLnk& AppLnk::operator=(const AppLnk &copy)
{
    if ( mId )
	qWarning("Deleting AppLnk that is in an AppLnkSet");
    if ( d )
	delete d;
    mName = copy.mName;
    mPixmap = copy.mPixmap;
    mBigPixmap = copy.mBigPixmap;
    mExec = copy.mExec;
    mType = copy.mType;
    mRotation = copy.mRotation;
    mComment = copy.mComment;
    mFile = copy.mFile;
    mLinkFile = copy.mLinkFile;
    mIconFile = copy.mIconFile;
    mMimeTypes = copy.mMimeTypes;
    mMimeTypeIcons = copy.mMimeTypeIcons;
    mId = 0;
    d = new AppLnkPrivate();
    d->mCat = copy.d->mCat.copy();
    d->mCatList = copy.d->mCatList;
    return *this;
}

/*!
  Returns a DocLnk that is a deep copy of \a other.

  First availability: Qtopia 1.6
*/

DocLnk & DocLnk::operator=(const DocLnk &other)
{
    AppLnk::operator=(other);
    return *this;
}

/*!
  Returns TRUE if the AppLnk is a DocLnk.

  First availability: Qtopia 1.6
*/

bool AppLnk::isDocLnk() const
{
    return type().contains('/'); // ###### need better predicate
}

void AppLnkSet::clear()
{
    QListIterator<AppLnk> it( mApps );
    for ( ; it.current(); ) {
	AppLnk* a = *it;
	++it;
	a->mId = 0;
	delete a;
    }
    mApps.clear();
    typs.clear();
}
