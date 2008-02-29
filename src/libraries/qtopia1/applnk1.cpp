//depot/qtopia/1/src/libraries/qtopia1/applnk1.cpp#1 - branch change 77716 (text)
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

#include <qtopia/applnk.h>

#include <qtopia/mimetype.h>
#include <qtopia/filemanager.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <stdlib.h>

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
    return mIconFile;
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



bool AppLnk::setLocation( const QString& docPath )
{
    //create it in default storage, if it doesn't exist.
    
    QString fileName = file();
    QString linkfileName = linkFile();
    
    AppLnk oldLnk = *this;
    
    //find unique names in the new location
    
    QFileInfo fi( fileName );
    
    QString baseName = docPath + "/" + type() + "/" + fi.baseName();
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

	int n = 1;
	QString nn;
	for(;;)
	    {
		nn = baseName + "_" + QString::number(n);
		if ( !QFile::exists( nn + ext ) &&
		     !QFile::exists( nn + ".desktop" ) ) {
		    baseName = nn;
		    break;
		}
		++n;
	    }
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
    d->mCat = copy.d->mCat;
    d->mCatList = copy.d->mCatList;
    return *this;
}


DocLnk & DocLnk::operator=(const DocLnk &other)
{
    AppLnk::operator=(other);
    return *this;
}

bool AppLnk::isDocLnk() const
{
    return type().contains('/'); // ###### need better predicate
}
