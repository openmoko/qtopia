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

#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qfile.h>

extern QStringList qpe_pathCache[Resource::AllResources + 1]; 

/*!
    \enum Resource::ResourceType
    
    \value PixResource 
    \value SoundResource
    \value AllResources
*/
    
/*!
    For a given resource type \a t append the paths contained in \a extras to resource search path. A given path choice may be either relative to $QPEDIR 
	or an absolute path

    First availability: Qtopia 2.0
*/
void Resource::setExtraPaths( ResourceType t, const QStringList &extras )
{
    QString resourcePath;
    qpe_pathCache[t].clear();
    for ( QStringList::ConstIterator prefix = extras.begin(); prefix!=extras.end(); ++prefix ){
	resourcePath = QPEApplication::qpeDir() + *prefix;
	if (QFile::exists( resourcePath )){
	    qpe_pathCache[t].append( resourcePath );
	} else {
	    if (QFile::exists( *prefix ))
		qpe_pathCache[t].append( *prefix );
	  else
	    qDebug( "Ignoring non-existant resource path of %s", resourcePath.local8Bit().data() );
	}
    }
}


/*!
    Returns a list of extra paths for given resource as absolute paths

    First availability: Qtopia 2.0
*/
const QStringList & Resource::extraPaths( ResourceType t ) 
{
    return qpe_pathCache[t];
}
