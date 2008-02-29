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
#include "waveform.h"

#include <qlabel.h>
#include <qpainter.h>


Waveform::Waveform( QWidget *parent, const char *name, WFlags fl )
    : QWidget( parent, name, fl )
{
    pixmap = 0;
    windowSize = 100;
    samplesPerPixel = 8000 / (5 * windowSize);
    currentValue = 0;
    numSamples = 0;
    windowPosn = 0;
    window = 0;
}


void Waveform::changeSettings( int frequency, int channels )
{
    makePixmap();
    samplesPerPixel = frequency * channels / (5 * windowSize);
    if ( !samplesPerPixel )
	samplesPerPixel = 1;
    currentValue = 0;
    numSamples = 0;
    windowPosn = 0;
    draw();
}


Waveform::~Waveform()
{
    if ( window )
	delete[] window;
    if ( pixmap )
	delete pixmap;
}


void Waveform::reset()
{
    makePixmap();
    currentValue = 0;
    numSamples = 0;
    windowPosn = 0;
    draw();
}


void Waveform::newSamples( const short *buf, int len )
{
    // Cache the object values in local variables.
    int samplesPerPixel = this->samplesPerPixel;
    int currentValue = this->currentValue;
    int numSamples = this->numSamples;
    short *window = this->window;
    int windowPosn = this->windowPosn;
    int windowSize = this->windowSize;

    // Average the incoming samples to scale them to the window.
    while ( len > 0 ) {
	currentValue += *buf++;
	--len;
	if ( ++numSamples >= samplesPerPixel ) {
	    window[windowPosn++] = (short)(currentValue / numSamples);
	    if ( windowPosn >= windowSize ) {
		this->windowPosn = windowPosn;
		draw();
		windowPosn = 0;
	    }
	    numSamples = 0;
	    currentValue = 0;
	}
    }

    // Copy the final state back to the object.
    this->currentValue = currentValue;
    this->numSamples = numSamples;
    this->windowPosn = windowPosn;
}


void Waveform::makePixmap()
{
    if ( !pixmap ) {
	pixmap = new QPixmap( size() );
	windowSize = pixmap->width();
	window = new short [windowSize];
    }
}


void Waveform::draw()
{
    pixmap->fill( Qt::black );
    QPainter painter;
    painter.begin( pixmap );
    painter.setPen( Qt::green );

    int middle = pixmap->height() / 2;
    int mag;
    short *window = this->window;
    int posn;
    int size = windowPosn;
    for( posn = 0; posn < size; ++posn )
    {
	mag = (window[posn] * middle / 32768);
	painter.drawLine(posn, middle - mag, posn, middle + mag);
    }
    if ( windowPosn < windowSize )
    {
	painter.drawLine(windowPosn, middle, windowSize, middle);
    }

    painter.end();

    paintEvent( 0 );
}


void Waveform::paintEvent( QPaintEvent * )
{
    QPainter painter;
    painter.begin( this );

    QSize sz = size();
    if ( pixmap && pixmap->size() != sz ) {
	delete pixmap;
	pixmap = 0;
	makePixmap();
	draw();
    }

    if ( pixmap ) {
	painter.drawPixmap( 0, 0, *pixmap );
    } else {
	painter.setPen( Qt::green );
	painter.drawLine(0, sz.height() / 2, sz.width(), sz.height() / 2);
    }

    painter.end();
}

