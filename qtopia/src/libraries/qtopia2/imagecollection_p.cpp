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

#include "imagecollection_p.h"

void ImageCollection::insert( const DocLnk& image )
{
    // Insert image into collection sorted by user visible name
    QValueList< DocLnk >::Iterator it( findGE( image ) );
    list.insert( it, image );
    emit inserted( image );
}

void ImageCollection::update( const DocLnk& image )
{
    // Remove image from collection
    QValueList< DocLnk >::Iterator it( find( image ) );
    list.remove( it );
    // Insert into collection sorted by user visible name and update current
    it = findGE( image );
    list_iterator = list.insert( it, image );
    emit updated( image );
}

void ImageCollection::remove( const DocLnk& image )
{
    // If image in collection, remove image from collection
    QValueList< DocLnk >::Iterator it( find( image ) );
    if( it != list.end() ) {
        list_iterator = list.remove( it );
        emit removed( image );
    }
    
    // If collection not empty, set current item to next item in collection
    if( !list.isEmpty() ) {
        if( list_iterator == list.end() )
            list_iterator = list.begin();
        emit currentChanged( *list_iterator );
    } 
}

void ImageCollection::clear()
{
    // Clear and initialize collection
    list.clear();
    list_iterator = list.begin();
    emit cleared();
}

DocLnk ImageCollection::currentImage() const
{
    if( list_iterator != list.end() ) 
        return *list_iterator;
    return DocLnk();
}

void ImageCollection::makeHeadCurrent()
{
    // If collection not empty, make head of list the current image
    if( !list.isEmpty() ) {
        list_iterator = list.begin();
        emit currentChanged( *list_iterator );
    }
}

void ImageCollection::setCurrentImage( const DocLnk& image )
{
    // If image not already current
    if( list_iterator != list.end() && 
        (*list_iterator).file() != image.file() ) {
        // Find image in collection and set iterator to image
        QValueList< DocLnk >::ConstIterator it( find( image ) );
        list_iterator = it;
    
        if( list_iterator != list.end() ) 
            emit currentChanged( *list_iterator );
    }
}

void ImageCollection::forward()
{
    // If there are images in the collection
    if( list.count() > 1 ) {
        // If current image at end of list, move to fist image
        // Otherwise, move current image forward one image
        list_iterator++;
        if( list_iterator == list.end() )
            list_iterator = list.begin();
        emit currentChanged( *list_iterator );
    }
}

void ImageCollection::back()
{
    // If there are images in the collection
    if( list.count() > 1 ) {
        // If current selection at start of list, move to last image
        // Otherwise, move current selection back one image
        if( list_iterator == list.begin() )
            list_iterator = list.end();
        list_iterator--;
        emit currentChanged( *list_iterator );
    }
}

QValueList< DocLnk >::Iterator ImageCollection::find( const DocLnk& image )
{
    QValueList< DocLnk >::Iterator it;
    for( it = list.begin(); it != list.end() && 
        (*it).file() != image.file(); ++it );
    return it;
}

QValueList< DocLnk >::Iterator ImageCollection::findGE( const DocLnk& image )
{
    QValueList< DocLnk >::Iterator it;
    for( it = list.begin(); it != list.end() &&
        (*it).name().lower().compare( image.name().lower() ) < 0; ++it );
    return it;
}
