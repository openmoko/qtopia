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
    if ( !QFile::exists(lf) ) {
	// May need to create directories
	QFileInfo fi(lf);
	QString dirPath = QDir::convertSeparators( fi.dirPath(TRUE) );

	QString param;
#ifndef Q_OS_WIN32
	param = "-p ";
#endif
	if ( system(("mkdir "+ param + dirPath.latin1()) ) )
	     return FALSE;
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
