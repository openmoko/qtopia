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
#include <stdlib.h>
#include "samplebuffer.h"


SampleBuffer::SampleBuffer( unsigned int _blockSize, unsigned int _maxBlocks )
{
    blockSize = _blockSize;
    maxBlocks = _maxBlocks;
    init();
}


SampleBuffer::~SampleBuffer()
{
    clearAll();
}


bool SampleBuffer::nextWriteBuffer( short *& buf, unsigned int& length )
{
    if ( writePosn ) {
	buf = writePosn->data;
	length = blockSize;
	return TRUE;
    } else {
	// The sample buffer ran out of memory or reached the block limit.
	return FALSE;
    }
}


void SampleBuffer::commitWriteBuffer ( unsigned int length )
{
    // Bail out if nothing to commit.
    if ( !writePosn || !length )
	return;

    // Update the current block size.
    writePosn->size = length;

    // Allocate space for the next block.
    if ( !maxBlocks || numBlocks < maxBlocks ) {

	BufferBlock *block;
	block = (BufferBlock *)malloc( sizeof(BufferBlock) +
				       ( blockSize - 1 ) * sizeof( short ) );
	if ( block ) {
	    writePosn->next = block;
	    block->next = 0;
	    block->size = 0;
	    writePosn = block;
	    ++numBlocks;
	} else {
	    // The process is out of memory.
	    writePosn = 0;
	}

    } else {
	// We've reached the maximum block limit.
	writePosn = 0;
    }
}


void SampleBuffer::rewind()
{
    readPosn = first;
}


bool SampleBuffer::nextReadBuffer( short *& buf, unsigned int& length )
{
    while ( readPosn ) {
	buf = readPosn->data;
	length = readPosn->size;
	readPosn = readPosn->next;
	if ( length > 0 )
	    return TRUE;
    }
    return FALSE;
}


void SampleBuffer::clear()
{
    clearAll();
    init();
}


void SampleBuffer::init()
{
    first = (BufferBlock *)malloc( sizeof(BufferBlock) +
				   ( blockSize - 1 ) * sizeof( short ) );
    readPosn = first;
    writePosn = first;
    if ( first ) {
	first->next = 0;
	first->size = 0;
	numBlocks = 1;
    } else {
	numBlocks = 0;
    }
}


void SampleBuffer::clearAll()
{
    BufferBlock *block, *next;
    block = first;
    while ( block ) {
	next = block->next;
	free( block );
	block = next;
    }
    first = 0;
    readPosn = 0;
    writePosn = 0;
    numBlocks = 0;
}

