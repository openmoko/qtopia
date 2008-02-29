/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

#include <qglobal.h>


class SampleBuffer
{
public:
    SampleBuffer( unsigned int blockSize, unsigned int maxBlocks = 0 );
    ~SampleBuffer();

    bool nextWriteBuffer( short *& buf, unsigned int& length );
    void commitWriteBuffer ( unsigned int length );
    void rewind();
    bool nextReadBuffer( short *& buf, unsigned int& length );
    void clear();

private:
    struct BufferBlock
    {
        struct BufferBlock *next;
        unsigned int size;
        short data[1];
    };

private:
    BufferBlock *first;
    BufferBlock *readPosn;
    BufferBlock *writePosn;
    unsigned int blockSize;
    unsigned int numBlocks;
    unsigned int maxBlocks;

    void init();
    void clearAll();
};


#endif

