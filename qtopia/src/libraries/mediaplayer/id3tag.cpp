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
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "id3tag.h"


/*!
  \class ID3Tag id3tag.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class reads and writes id3 tags from media file.
*/


ID3Tag::ID3Tag()
{
}


QString ID3Tag::toString()
{
    QString info;
    QString label[6] = { tr( "Title" ), tr( "Artist" ), tr( "Album" ), tr( "Year" ), tr( "Comment" ), tr( "Track" ) };
    bool first = TRUE;
    for ( int i = 0; i < 6; i++ ) {
	if ( !idFields[i].isEmpty() ) {
	    info += ( first ? "" : ", " ) + label[i] + ": " + (const char *)idFields[i];
	    first = FALSE;
	}
    }
    return info;
}


bool ID3Tag::read( int fd )
{
    uchar id3v1[128 + 1];
    bool isGood = TRUE;

    // get the original file position
    off_t originalPosition = ::lseek( fd, 0L, SEEK_CUR );

    // seek to the position in the file where an ID tag is expected
    if ( ::lseek( fd, -128, SEEK_END ) == -1 ) {
	qDebug( "error seeking to id3 tags" );
	isGood = FALSE;
    }

    // read in the id3 tag data
    if ( isGood && ::read( fd, id3v1, 128 ) != 128 ) {
	qDebug( "error reading id3 tags" );
	isGood = FALSE;
    }

    // check the tag header
    if ( isGood && ::strncmp( (const char *)id3v1, "TAG", 3 ) != 0 ) {
	// No id3 tags
	isGood = FALSE;
    } else if ( isGood ) {
        // parse the tag fields
	int len[5] = { 30, 30, 30, 4, 30 };
	char *ptr = (char *)id3v1 + 3, *ptr2 = ptr + len[0];
	for ( int i = 0; i < 5; ptr += len[i], i++, ptr2 += len[i] ) {
	    char push = *ptr2;
	    *ptr2 = '\0';
	    char *ptr3 = ptr2;
	    while ( ptr3-1 >= ptr && isspace(ptr3[-1]) ) ptr3--;
	    char push2 = *ptr3; *ptr3 = '\0';
	    if ( strcmp( ptr, "" ) ) 
		idFields[i] = ptr;
	    *ptr3 = push2;
	    *ptr2 = push;
	}
/*
	// Debug ID3 Tags
	// --------------
	for ( int i = 0; i < 64; i++ )
	    printf("%i", i % 10 );
	printf("\n");
	for ( int i = 0; i < 64; i++ )
	    printf("%c", ( isprint( id3v1[i] ) != 0 ) ? id3v1[i] : '-' );
	printf("\n");
	for ( int i = 0; i < 64; i++ )
	    printf("%i", i % 10 );
	printf("\n");
	for ( int i = 0; i < 64; i++ )
	    printf("%c", ( isprint( id3v1[i+64] ) != 0 ) ? id3v1[i+64] : '-' );
	printf("\n");
*/
	idFields[5] = (id3v1[127] == 255 && id3v1[126] != 0) ? QString::number( id3v1[126] ) : QString("");
    }

    // reset the file position to the original position 
    ::lseek( fd, originalPosition, SEEK_SET );
    return isGood;
}


bool ID3Tag::write( int fd )
{
    uchar id3v1[128 + 1];
    bool isGood = TRUE;

    // get the original file position
    off_t originalPosition = ::lseek( fd, 0L, SEEK_CUR );

    // seek to the position in the file where an ID tag is expected
    if ( ::lseek( fd, -128, SEEK_END ) == -1 || ::read( fd, id3v1, 128 ) != 128 ) {
	qDebug( "error reading id3 tags" );
	isGood = FALSE;
    } else {
	// check the tag header to see if it has tags, else we must append the new tags to the end
	if ( ::strncmp( (const char *)id3v1, "TAG", 3 ) != 0 ) {
	    if ( ::lseek( fd, 0, SEEK_END ) == -1 ) {
		qDebug( "error seeking to end of file to output new tags" );
		isGood = FALSE;
	    }
	} else {
	    // We have old tags, so rewind to where they start to overwrite them
	    if ( ::lseek( fd, -128, SEEK_END ) == -1 ) {
		qDebug( "error seeking to beginning of old tags to overwrite" );
		isGood = FALSE;
	    }
	}
    }

    if ( isGood ) {
	// build the id3 data structure
	id3v1[0] = 'T', id3v1[1] = 'A', id3v1[2] = 'G';
	uint i = 3;
	uint len[5] = { 30, 30, 30, 4, 30 };
	for ( uint t = 0; t < sizeof(len)/sizeof(len[0]); t++ ) {
	    int copyLength = QMIN( idFields[t].length(), len[t] );
	    int remaining = len[t] - copyLength;
	    strncpy( (char *)id3v1 + i, (const char *)idFields[t], copyLength );
	    memset( (char *)id3v1 + i + copyLength, 0, remaining ); // fill remaining with zeros
	    i += len[t];
	}
	// Track field
	id3v1[125] = 0;
	id3v1[126] = idFields[5].toUInt();
	id3v1[127] = 255;
	// Write it out
	::write( fd, id3v1, 128 );
    }

    // reset the file position to the original position 
    ::lseek( fd, originalPosition, SEEK_SET );
    return isGood;
}

