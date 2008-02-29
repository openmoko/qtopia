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
#ifndef CARDMETRICS_H
#define CARDMETRICS_H


#include <qsize.h>
class QPixmap;
class QBitmap;


// Global Card Metrics
class CardMetrics {
    public:
	enum CardSize {
	    micro,
	    small,
	    normal
	};

	static void loadMetrics( int width, int height );
	static void setCardBack( int back );

	static int width() { return w; };
	static int height() { return h; };
	static QSize size() { return QSize(w, h); }
	static int interCardGap() { return g; }
	static int xOffset() { return xoff; }
	static int metric(int i) { return m[i]; }
	static const char *path();

	static int suitWidth() { return sw; }
	static int suitHeight() { return sh; }
	static int suitSmallWidth() { return ssw; }
	static int suitSmallHeight() { return ssh; }
	static int rankWidth() { return rw; }
	static int rankHeight() { return rh; }
	static int spadeWidth() { return spw; }
	static int spadeHeight() { return sph; }
	static int offsetDown() { return od; } // 9 + 4 * cardSizes; }

	static QPixmap *facePixmap() { return cardsFaces; }
	static QPixmap *backPixmap() { return cardsBacks; }

	static QPixmap *jokerPixmap() { return cardsJoker; }
	static QPixmap *spadePixmap() { return cardsSpade; }

	static QPixmap *suitPixmap() { return cardsSuits; }
	static QPixmap *suitIPixmap() { return cardsSuitsUpsideDown; }

	static QPixmap *suitSmPixmap() { return cardsSuitsSmall; }
	static QPixmap *suitSmIPixmap() { return cardsSuitsSmallUpsideDown; }

	static QPixmap *rankPixmap() { return cardsChars; }
	static QPixmap *rankBigPixmap() { return cardsBigChars; }
	static QPixmap *rankIPixmap() { return cardsCharsUpsideDown; }

	static QPixmap *picturePixmap() { return cardsPictures; }
	static QPixmap *pictureIPixmap() { return cardsPicturesUpsideDown; }

//	static int cardHeight() = 27 + 9 * cardSizes;
//	static int cardWidth()  = 20 + 3 * cardSizes;

    private:
	static int cardBack;
	static int w, h, g, m[100];
	static CardSize cardSizes;
	static int xoff;
	static int sw; 
	static int sh; 
	static int ssw; 
	static int ssh; 
	static int rw; 
	static int rh; 
	static int od; 
	static int spw; 
	static int sph; 

	static QPixmap *cardsFaces;
	static QPixmap *cardsBacks;
	static QPixmap *cardsJoker;
	static QPixmap *cardsSpade;
	static QPixmap *cardsPictures;
	static QPixmap *cardsPicturesUpsideDown;

	static QPixmap *cardsChars;
	static QPixmap *cardsBigChars;
	static QPixmap *cardsSuits;
	static QPixmap *cardsSuitsSmall;
	static QPixmap *cardsCharsUpsideDown;
	static QPixmap *cardsSuitsUpsideDown;
	static QPixmap *cardsSuitsSmallUpsideDown;
};


#endif

