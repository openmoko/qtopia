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
#include "cardmetrics.h"
#include <qwmatrix.h>
#include <qpixmap.h>
#include <qtopia/resource.h>
#include <qlabel.h>


QPixmap *Create180RotatedPixmap(QPixmap *srcPix)
{
#ifdef QT_NO_TRANSFORMATIONS
    int w = srcPix->width();
    int h = srcPix->height();
    QPixmap *dstPix = new QPixmap( w, h );
    // ### this is very poorly implemented and probably could be much faster
    for (int i = 0; i < w; i++)
	for (int j = 0; j < h; j++)
	    bitBlt( dstPix, i, j, srcPix, w - i - 1, h - j - 1, 1, 1 );
    return dstPix;
#else
    QWMatrix m;
    m.rotate( 180.0 );
    return new QPixmap( srcPix->xForm( m ) );
#endif
}


QBitmap *Create180RotatedBitmap(QBitmap *srcPix)
{
#ifdef QT_NO_TRANSFORMATIONS
    int w = srcPix->width();
    int h = srcPix->height();
    QBitmap *dstPix = new QBitmap( w, h );
    // ### this is very poorly implemented and probably could be much faster
    for (int i = 0; i < w; i++)
	for (int j = 0; j < h; j++)
	    bitBlt( dstPix, i, j, srcPix, w - i - 1, h - j - 1, 1, 1 );
    return dstPix;
#else
    QWMatrix m;
    m.rotate( 180.0 );
    return new QBitmap( srcPix->xForm( m ) );
#endif
}


// Determines if we resize to mega small on really low res screens
CardMetrics::CardSize CardMetrics::cardSizes = CardMetrics::small;
int CardMetrics::w = 0;
int CardMetrics::h = 0;
int CardMetrics::g = 0;
int CardMetrics::m[100];
int CardMetrics::xoff = 0;
int CardMetrics::sw; 
int CardMetrics::sh; 
int CardMetrics::ssw; 
int CardMetrics::ssh; 
int CardMetrics::rw; 
int CardMetrics::rh; 
int CardMetrics::od;
int CardMetrics::spw; 
int CardMetrics::sph; 
int CardMetrics::cardBack = 0; 


// Initialise static member variables to NULL
QPixmap *CardMetrics::cardsFaces = NULL;
QPixmap *CardMetrics::cardsBacks = NULL;
QPixmap *CardMetrics::cardsJoker = NULL;
QPixmap *CardMetrics::cardsSpade = NULL;
QPixmap *CardMetrics::cardsPictures = NULL;
QPixmap *CardMetrics::cardsPicturesUpsideDown = NULL;
QPixmap *CardMetrics::cardsChars = NULL;
QPixmap *CardMetrics::cardsBigChars = NULL;
QPixmap *CardMetrics::cardsSuits = NULL;
QPixmap *CardMetrics::cardsSuitsSmall = NULL;
QPixmap *CardMetrics::cardsCharsUpsideDown = NULL;
QPixmap *CardMetrics::cardsSuitsUpsideDown = NULL;
QPixmap *CardMetrics::cardsSuitsSmallUpsideDown = NULL;


static const char *cardSizePaths[] = { "micro", "small", "normal" }; // No tr

/*
void CardMetrics::drawSuitShape( QPainter &p, int x, int y, int suit, bool inverted ) {
    if ( !inverted )
        p->drawPixmap( x, y, *suitPixmap(), sw * (suit + 1), 0, sw, sh );
    else
	p->drawPixmap( x, y, *suitIPixmap(), sw * (3 - suit + 1), 0, sw, sh );
}
*/

void CardMetrics::setCardBack( int b )
{
    if ( cardBack != b ) {
	cardBack = b;
	if ( cardsBacks )
	    delete cardsBacks;
	QString cardImagePath;
	cardImagePath = cardImagePath.sprintf( "cards/%s/back%02i", CardMetrics::path(), cardBack + 1 );
	cardsBacks = new QPixmap( Resource::loadPixmap( cardImagePath ) );
    }
}

void CardMetrics::loadMetrics( int width, int height ) {
   
    int oldSize = cardSizes;

    int retry = 0;
 
retry_smaller_size:
    cardSizes = CardMetrics::CardSize((( width < 230 ) ? 0 :
		    ( width < 450 ) ? 1 : 2)-retry);

    if ( (int)cardSizes < 0 )
	qFatal("No card images found");

    if ( cardsFaces == NULL || cardSizes != oldSize ) {
	int cardSizesGaps[] = { 3, 7, 10 };
	g = cardSizesGaps[cardSizes];
    
	QString cardImagePath = QString("cards/") + CardMetrics::path() + "/";

	delete cardsFaces;
	cardsFaces = new QPixmap( Resource::loadPixmap( cardImagePath + "face" ) );

	if ( cardsFaces->isNull() ) {
	    retry++;
	    delete cardsFaces;
	    cardsFaces = 0;
	    goto retry_smaller_size;
	}

	QString back;
	delete cardsBacks;
	cardsBacks = new QPixmap( Resource::loadPixmap( cardImagePath + back.sprintf( "back%02i", cardBack + 1 ) ) );
	delete cardsJoker;
	cardsJoker = new QPixmap( Resource::loadPixmap( cardImagePath + "joker" ) );

	delete cardsPictures;
	cardsPictures = new QPixmap( Resource::loadPixmap( cardImagePath + "pictures" ) );
#ifndef QTOPIA_PHONE
	cardsPicturesUpsideDown = Create180RotatedPixmap( cardsPictures );
#endif

	QBitmap *tCardsSpade = new QBitmap( Resource::loadBitmap( cardImagePath + "spade" ) );
	delete cardsSpade;
	cardsSpade = new QPixmap( *tCardsSpade );
	cardsSpade->setMask( *tCardsSpade );
	delete tCardsSpade;

#ifdef QTOPIA_PHONE
	QBitmap *tCardsBigChars = new QBitmap( Resource::loadBitmap( "cards/normal/ranks" ) ); // No tr
	delete cardsBigChars;
	cardsBigChars = new QPixmap( *tCardsBigChars );
	cardsBigChars->setMask( *tCardsBigChars );
	delete tCardsBigChars;
#endif

	QBitmap *tCardsChars = new QBitmap( Resource::loadBitmap( cardImagePath + "ranks" ) ); // No tr
#ifndef QTOPIA_PHONE
	QBitmap *tCardsCharsUpsideDown = Create180RotatedBitmap( tCardsChars );
#endif
	delete cardsChars;
	cardsChars = new QPixmap( *tCardsChars );
	cardsChars->setMask( *tCardsChars );
#ifndef QTOPIA_PHONE
	delete cardsCharsUpsideDown;
	cardsCharsUpsideDown = new QPixmap( *tCardsCharsUpsideDown );
	cardsCharsUpsideDown->setMask( *tCardsCharsUpsideDown );
	delete tCardsCharsUpsideDown;
#endif
	delete tCardsChars;

	QBitmap *tCardsSuitsSmall = new QBitmap( Resource::loadBitmap( cardImagePath + "suits01" ) ); // No tr
#ifndef QTOPIA_PHONE
	QBitmap *tCardsSuitsSmallUpsideDown = Create180RotatedBitmap( tCardsSuitsSmall );
#endif
	delete cardsSuitsSmall;
	cardsSuitsSmall = new QPixmap( *tCardsSuitsSmall );
	cardsSuitsSmall->setMask( *tCardsSuitsSmall );
#ifndef QTOPIA_PHONE
	delete cardsSuitsSmallUpsideDown;
	cardsSuitsSmallUpsideDown = new QPixmap( *tCardsSuitsSmallUpsideDown );
	cardsSuitsSmallUpsideDown->setMask( *tCardsSuitsSmallUpsideDown );
	delete tCardsSuitsSmallUpsideDown;
#endif
	delete tCardsSuitsSmall;

	QBitmap *tCardsSuits = new QBitmap( Resource::loadBitmap( cardImagePath + "suits02" ) ); // No tr
#ifndef QTOPIA_PHONE
	QBitmap *tCardsSuitsUpsideDown = Create180RotatedBitmap( tCardsSuits );
#endif
	delete cardsSuits;
	cardsSuits = new QPixmap( *tCardsSuits );
	cardsSuits->setMask( *tCardsSuits );
#ifndef QTOPIA_PHONE
	delete cardsSuitsUpsideDown;
	cardsSuitsUpsideDown = new QPixmap( *tCardsSuitsUpsideDown );
	cardsSuitsUpsideDown->setMask( *tCardsSuitsUpsideDown );
	delete tCardsSuitsUpsideDown;
#endif
	delete tCardsSuits;

	w = cardsFaces->width();
	h = cardsFaces->height();
	sw = cardsSuits->width() / 4; 
	sh = cardsSuits->height(); 
	ssw = cardsSuitsSmall->width() / 4; 
	ssh = cardsSuitsSmall->height(); 
	rw = cardsChars->width() / 13; 
	rh = cardsChars->height(); 
	spw = cardsSpade->width(); 
	sph = cardsSpade->height();

	cardsFaces->setOptimization( QPixmap::BestOptim );
	cardsBacks->setOptimization( QPixmap::BestOptim );
	cardsJoker->setOptimization( QPixmap::BestOptim );

	cardsPictures->setOptimization( QPixmap::BestOptim );
#ifndef QTOPIA_PHONE
	cardsPicturesUpsideDown->setOptimization( QPixmap::BestOptim );
#endif

	cardsChars->setOptimization( QPixmap::BestOptim );
#ifndef QTOPIA_PHONE
	cardsCharsUpsideDown->setOptimization( QPixmap::BestOptim );
#endif

	cardsSuitsSmall->setOptimization( QPixmap::BestOptim );
#ifndef QTOPIA_PHONE
	cardsSuitsSmallUpsideDown->setOptimization( QPixmap::BestOptim );
#endif

	cardsSuits->setOptimization( QPixmap::BestOptim );
#ifndef QTOPIA_PHONE
	cardsSuitsUpsideDown->setOptimization( QPixmap::BestOptim );
#endif
    }
    od = rh + ssh / 2 + 4;
    while (od > rh+2 && h*2+od*16 > height ) {
	od--;
    }
 
    int totalWidth = 7 * w + 6 * g;
    int spareWidth = width - totalWidth;
    xoff = spareWidth / 2;
}


const char *CardMetrics::path() {
    return cardSizePaths[cardSizes];
}



