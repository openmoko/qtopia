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
#include "cardmetrics.h"
#include <qwmatrix.h>
#include <qpixmap.h>
#include <qpe/resource.h>
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
QPixmap *CardMetrics::cardsSuits = NULL;
QPixmap *CardMetrics::cardsSuitsSmall = NULL;
QPixmap *CardMetrics::cardsCharsUpsideDown = NULL;
QPixmap *CardMetrics::cardsSuitsUpsideDown = NULL;
QPixmap *CardMetrics::cardsSuitsSmallUpsideDown = NULL;


static const char *cardSizePaths[] = { "micro", "small", "normal", "large" };

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

void CardMetrics::loadMetrics( int width, int ) {
   
    int oldSize = cardSizes;
 
    cardSizes = ( width < 220 ) ? micro :
		    ( width < 500 ) ? small :
			( width < 1000 ) ? normal :
			    large;

    if ( cardsFaces == NULL || cardSizes != oldSize ) {

	int cardSizesGaps[] = { 2, 7, 10, 30 };
	g = cardSizesGaps[cardSizes];
    
	QString cardImagePath = QString("cards/") + CardMetrics::path() + "/";

	cardsFaces = new QPixmap( Resource::loadPixmap( cardImagePath + "face" ) );
	QString back;
	cardsBacks = new QPixmap( Resource::loadPixmap( cardImagePath + back.sprintf( "back%02i", cardBack + 1 ) ) );
	cardsJoker = new QPixmap( Resource::loadPixmap( cardImagePath + "joker" ) );

	cardsPictures = new QPixmap( Resource::loadPixmap( cardImagePath + "pictures" ) );
	cardsPicturesUpsideDown = Create180RotatedPixmap( cardsPictures );

	QBitmap *tCardsSpade = new QBitmap( Resource::loadBitmap( cardImagePath + "spade" ) );
	cardsSpade = new QPixmap( *tCardsSpade );
	cardsSpade->setMask( *tCardsSpade );
	delete tCardsSpade;

	QBitmap *tCardsChars = new QBitmap( Resource::loadBitmap( cardImagePath + "ranks" ) );
	QBitmap *tCardsCharsUpsideDown = Create180RotatedBitmap( tCardsChars );
	cardsChars = new QPixmap( *tCardsChars );
	cardsChars->setMask( *tCardsChars );
	cardsCharsUpsideDown = new QPixmap( *tCardsCharsUpsideDown );
	cardsCharsUpsideDown->setMask( *tCardsCharsUpsideDown );
	delete tCardsChars;
	delete tCardsCharsUpsideDown;

	QBitmap *tCardsSuitsSmall = new QBitmap( Resource::loadBitmap( cardImagePath + "suits01" ) );
	QBitmap *tCardsSuitsSmallUpsideDown = Create180RotatedBitmap( tCardsSuitsSmall );
	cardsSuitsSmall = new QPixmap( *tCardsSuitsSmall );
	cardsSuitsSmall->setMask( *tCardsSuitsSmall );
	cardsSuitsSmallUpsideDown = new QPixmap( *tCardsSuitsSmallUpsideDown );
	cardsSuitsSmallUpsideDown->setMask( *tCardsSuitsSmallUpsideDown );
	delete tCardsSuitsSmall;
	delete tCardsSuitsSmallUpsideDown;

	QBitmap *tCardsSuits = new QBitmap( Resource::loadBitmap( cardImagePath + "suits02" ) );
	QBitmap *tCardsSuitsUpsideDown = Create180RotatedBitmap( tCardsSuits );
	cardsSuits = new QPixmap( *tCardsSuits );
	cardsSuits->setMask( *tCardsSuits );
	cardsSuitsUpsideDown = new QPixmap( *tCardsSuitsUpsideDown );
	cardsSuitsUpsideDown->setMask( *tCardsSuitsUpsideDown );
	delete tCardsSuits;
	delete tCardsSuitsUpsideDown;

	w = cardsFaces->width();
	h = cardsFaces->height();
	sw = cardsSuits->width() / 4; 
	sh = cardsSuits->height(); 
	ssw = cardsSuitsSmall->width() / 4; 
	ssh = cardsSuitsSmall->height(); 
	rw = cardsChars->width() / 13; 
	rh = cardsChars->height(); 
	od = cardsChars->height() + ssh / 2 + 4;
	spw = cardsSpade->width(); 
	sph = cardsSpade->height();

    }
 
    int totalWidth = 7 * w + 6 * g;
    int spareWidth = width - totalWidth;
    xoff = spareWidth / 2;
}


const char *CardMetrics::path() {
    return cardSizePaths[cardSizes];
}



