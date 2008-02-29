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

#ifndef IMAGECOLLECTION_P_H
#define IMAGECOLLECTION_P_H

#include <qtopia/applnk.h>

#include <qobject.h>
#include <qvaluelist.h>

class ImageCollection : public QObject
{
    Q_OBJECT
public:
    // Return image collection
    const QValueList< DocLnk >& collection() const { return list; }
    
    // Insert image into collection sorted by user visible name
    void insert( const DocLnk& );
    
    // Update image in collection while maintaing order by user visible name
    void update( const DocLnk& );
    
    // Remove image from collection
    void remove( const DocLnk& );
    
    // Clear collection
    void clear();
    
    // Return true if collection empty
    bool isEmpty() { return list.isEmpty(); } 
    
    // Return the current image
    DocLnk currentImage() const;
    
signals:
    // Image inserted into collection
    void inserted( const DocLnk& );
    
    // Image updated in collection
    void updated( const DocLnk& );
    
    // Image removed from collection
    void removed( const DocLnk& );
    
    // Collection cleared
    void cleared();
    
    // Current image has been set
    void currentChanged( const DocLnk& );
    
public slots:
    // Make head of collection current
    void makeHeadCurrent();
    
    // Set the current image
    void setCurrentImage( const DocLnk& );
    
    // Move current image forward one image
    void forward();

    // Move current image back one image
    void back();
    
private:
    // Find image in collection using filename as key
    QValueList< DocLnk >::Iterator find( const DocLnk& );
    
    // Find first image in collection with a user visible name >= image
    QValueList< DocLnk >::Iterator findGE( const DocLnk& );

    QValueList< DocLnk > list;
    QValueList< DocLnk >::ConstIterator list_iterator;
};

#endif
