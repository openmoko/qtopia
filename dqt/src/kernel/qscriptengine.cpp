/****************************************************************************
** $Id: qt/qscriptengine.cpp   3.3.5   edited Aug 31 12:13 $
**
** Copyright (C) 2003-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qscriptengine_p.h"

#include "qstring.h"
#include "qrect.h"
#include "qfont.h"
#include <private/qunicodetables_p.h>
#include "qtextengine_p.h"
#include "qfontengine_p.h"
#include <stdlib.h>


#undef None
#undef Pre
#undef Above
#undef Below

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Basic processing
//
// --------------------------------------------------------------------------------------------------------------------------------------------

static inline void positionCluster( QTextEngine *engine, QScriptItem *si, int gfrom,  int glast )
{
    si->hasPositioning = TRUE;
    int nmarks = glast - gfrom;
    if ( nmarks <= 0 ) {
	qWarning( "positionCluster: no marks to position!" );
	return;
    }

    glyph_t *glyphs = engine->glyphs( si );
    qoffset_t *offsets = engine->offsets( si );
    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );

    QFontEngine *f = si->fontEngine;
    glyph_metrics_t baseInfo = f->boundingBox( glyphs[gfrom] );

    if ( si->analysis.script == QFont::Hebrew ) {
	// we need to attach below the baseline, because of the hebrew iud.
	baseInfo.height= QMAX( baseInfo.height, -baseInfo.y );
    }
    QRect baseRect( baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height );

//     qDebug("---> positionCluster: cluster from %d to %d", gfrom, glast );
//     qDebug( "baseInfo: %d/%d (%d/%d) off=%d/%d", baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height, baseInfo.xoff, baseInfo.yoff );

    int size = f->ascent()/10;
    int offsetBase = (size - 4) / 4 + QMIN( size, 4 ) + 1;
//     qDebug("offset = %d", offsetBase );

    bool rightToLeft = si->analysis.bidiLevel % 2;

    int i;
    int lastCmb = -1;
    QRect attachmentRect;

    for( i = 1; i <= nmarks; i++ ) {
	glyph_t mark = glyphs[gfrom+i];
	QPoint p;
	glyph_metrics_t markInfo = f->boundingBox( mark );
	QRect markRect( markInfo.x, markInfo.y, markInfo.width, markInfo.height );

	int offset = offsetBase;
	unsigned char cmb = glyphAttributes[gfrom+i].combiningClass;

	// ### maybe the whole position determination should move down to heuristicSetGlyphAttributes. Would save some
	// bits  in the glyphAttributes structure.
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew, lao and thai.

	    // for Lao and Thai marks with class 0, see below ( heuristicSetGlyphAttributes )

	    // add a bit more offset to arabic, a bit hacky
	    if ( cmb >= 27 && cmb <= 36 && offset < 3 )
		offset +=1;
 	    // below
	    if ( (cmb >= 10 && cmb <= 18) ||
		 cmb == 20 || cmb == 22 ||
		 cmb == 29 || cmb == 32 )
		cmb = QChar::Combining_Below;
	    // above
	    else if ( cmb == 23 || cmb == 27 || cmb == 28 ||
		      cmb == 30 || cmb == 31 || (cmb >= 33 && cmb <= 36 ) )
		cmb = QChar::Combining_Above;
	    //below-right
	    else if ( cmb == 9 || cmb == 103 || cmb == 118 )
		cmb = QChar::Combining_BelowRight;
	    // above-right
	    else if ( cmb == 24 || cmb == 107 || cmb == 122 )
		cmb = QChar::Combining_AboveRight;
	    else if ( cmb == 25 )
		cmb = QChar::Combining_AboveLeft;
	    // fixed:
	    //  19 21

	}

	// combining marks of different class don't interact. Reset the rectangle.
	if ( cmb != lastCmb ) {
	    //qDebug( "resetting rect" );
	    attachmentRect = baseRect;
	}

	switch( cmb ) {
	case QChar::Combining_DoubleBelow:
		// ### wrong in rtl context!
	case QChar::Combining_BelowLeft:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowLeftAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    break;
	case QChar::Combining_Below:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_BelowRight:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowRightAttached:
	    p += attachmentRect.bottomRight() - markRect.topRight();
	    break;
	    case QChar::Combining_Left:
	    p += QPoint( -offset, 0 );
	case QChar::Combining_LeftAttached:
	    break;
	    case QChar::Combining_Right:
	    p += QPoint( offset, 0 );
	case QChar::Combining_RightAttached:
	    break;
	case QChar::Combining_DoubleAbove:
	    // ### wrong in RTL context!
	case QChar::Combining_AboveLeft:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveLeftAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    break;
	    case QChar::Combining_Above:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_AboveRight:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveRightAttached:
	    p += attachmentRect.topRight() - markRect.bottomRight();
	    break;

	case QChar::Combining_IotaSubscript:
	    default:
		break;
	}
// 	qDebug( "char=%x combiningClass = %d offset=%d/%d", mark, cmb, p.x(), p.y() );
	markRect.moveBy( p.x(), p.y() );
	attachmentRect |= markRect;
	lastCmb = cmb;
	if ( rightToLeft ) {
	    offsets[gfrom+i].x = p.x();
	    offsets[gfrom+i].y = p.y() - baseInfo.yoff;
	} else {
	    offsets[gfrom+i].x = p.x() - baseInfo.xoff;
	    offsets[gfrom+i].y = p.y() - baseInfo.yoff;
	}
    }
}


void q_heuristicPosition( QTextEngine *engine, QScriptItem *si )
{
    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );

    int cEnd = -1;
    int i = si->num_glyphs;
    while ( i-- ) {
	if ( cEnd == -1 && glyphAttributes[i].mark ) {
	    cEnd = i;
	} else if ( cEnd != -1 && !glyphAttributes[i].mark ) {
	    positionCluster( engine, si, i, cEnd );
	    cEnd = -1;
	}
    }
}



// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars ang glyphs
// and no reordering.
// also computes logClusters heuristically
static void heuristicSetGlyphAttributes( const QChar *uc, int len,
					 QTextEngine *engine, QScriptItem *si )
{
    // ### zeroWidth and justification are missing here!!!!!

    if ( si->num_glyphs != len )
	qWarning("QScriptEngine::heuristicSetGlyphAttributes: char length and num glyphs disagree" );

//     qDebug("QScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", num_glyphs);
    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );
    unsigned short *logClusters = engine->logClusters( si );
    advance_t *advances = engine->advances( si );

    // honour the logClusters array if it exists.
    for ( int i = 0; i < si->num_glyphs; i++ )
	logClusters[i] = i;

    int pos = 1;

    // first char in a run is never (treated as) a mark
    int cStart = 0;
    glyphAttributes[0].mark = FALSE;
    glyphAttributes[0].clusterStart = TRUE;

    while ( pos < len ) {
	if ( ::category( uc[pos] ) != QChar::Mark_NonSpacing ) {
	    glyphAttributes[pos].mark = FALSE;
	    glyphAttributes[pos].clusterStart = TRUE;
	    glyphAttributes[pos].combiningClass = 0;
	    cStart = pos;
	} else {
	    int cmb = combiningClass( uc[pos] );

	    if ( cmb == 0 ) {
		// Fix 0 combining classes
		if ( uc[pos].row() == 0x0e ) {
		    // thai or lao
		    unsigned char col = uc[pos].cell();
		    if ( col == 0x31 ||
			 col == 0x34 ||
			 col == 0x35 ||
			 col == 0x36 ||
			 col == 0x37 ||
			 col == 0x47 ||
			 col == 0x4c ||
			 col == 0x4d ||
			 col == 0x4e ) {
			cmb = QChar::Combining_AboveRight;
		    } else if ( col == 0xb1 ||
				col == 0xb4 ||
				col == 0xb5 ||
				col == 0xb6 ||
				col == 0xb7 ||
				col == 0xbb ||
				col == 0xcc ||
				col == 0xcd ) {
			cmb = QChar::Combining_Above;
		    } else if ( col == 0xbc ) {
			cmb = QChar::Combining_Below;
		    }
		}
	    }

	    glyphAttributes[pos].mark = TRUE;
	    glyphAttributes[pos].clusterStart = FALSE;
	    glyphAttributes[pos].combiningClass = cmb;
	    // 		qDebug("found a mark at position %d", pos );
	    logClusters[pos] = cStart;
	    advances[pos] = 0;
	    si->hasPositioning = TRUE;
	}
	pos++;
    }
}

static void heuristicSetGlyphAttributes( const QString &string, int from, int len,
					 QTextEngine *engine, QScriptItem *si )
{
    heuristicSetGlyphAttributes(string.unicode() + from, len, engine, si);
}


static void convertToCMap( const QChar *chars, int len, QTextEngine *engine, QScriptItem *si )
{
    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );

    si->num_glyphs = len;
    engine->ensureSpace( len );
    int error = si->fontEngine->stringToCMap( chars, len, glyphs, advances, &si->num_glyphs, (si->analysis.bidiLevel %2) );
    if ( error == QFontEngine::OutOfMemory ) {
	engine->ensureSpace( si->num_glyphs );
	si->fontEngine->stringToCMap( chars, len, glyphs, advances, &si->num_glyphs, (si->analysis.bidiLevel %2) );
    }
}

static void basic_shape( int /*script*/, const QString &string, int from, int len, QTextEngine *engine, QScriptItem *si )
{
    convertToCMap( string.unicode() + from, len, engine, si );
    heuristicSetGlyphAttributes( string, from, len, engine, si );
    if ( !engine->widthOnly ) {
	q_heuristicPosition( engine, si );
    }
}

static void basic_attributes( int /*script*/, const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    attributes += from;

    QCharAttributes *a = attributes;

    for ( int i = 0; i < len; i++ ) {
	QChar::Category cat = ::category( *uc );
	a->whiteSpace = (cat == QChar::Separator_Space) && (uc->unicode() != 0xa0);
	a->softBreak = FALSE;
	a->charStop = (cat != QChar::Mark_NonSpacing);
	a->wordStop = FALSE;
	a->invalid = FALSE;
	++uc;
	++a;
    }
}

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
static void unicode_shape( int /*script*/, const QString &string, int from, int len, QTextEngine *engine, QScriptItem *si )
{
    QString s = string.mid(from, len);
    QChar *c = (QChar *)s.unicode();
    int i;
    for (i = 0; i < len; ++i) {
        ushort uc = c[i].unicode();
        if (uc >= 0x200e && uc < 0x2070) {
            if (uc < 0x2010
                || (uc >= 0x2028 && uc <= 0x202f)
                || uc >= 0x206a)
                c[i] = QChar(0x20);
        }
    }
    convertToCMap( s.unicode(), len, engine, si );
    const QChar *cc = string.unicode() + from;
    advance_t *a = engine->advances(si);
    for (i = 0; i < len; ++i) {
        ushort uc = cc[i].unicode();
        if (uc >= 0x200e && uc < 0x2070) {
            if (uc < 0x2010
                || (uc >= 0x2028 && uc <= 0x202f)
                || uc >= 0x206a)
                a[i] = 0;
        }
    }
    heuristicSetGlyphAttributes( string, from, len, engine, si );
    if ( !engine->widthOnly ) {
	q_heuristicPosition( engine, si );
    }
}
#endif



// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Middle eastern languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------

/* Hebrew shaping. In the non opentype case we try to use the
   presentation forms specified for Hebrew. Especially for the
   ligatures with Dagesh this gives much better results than we could
   achieve manually.
*/
static void hebrew_shape(int script, const QString &string, int from, int len,
			 QTextEngine *engine, QScriptItem *si)
{
    assert(script == QFont::Hebrew);

    if (len == 0)
        return;
    
#if defined( Q_WS_X11) && !defined( QT_NO_XFTFREETYPE )
    QOpenType *openType = si->fontEngine->openType();

    if ( openType && openType->supportsScript( script ) ) {
	convertToCMap( string.unicode() + from, len, engine, si );
	heuristicSetGlyphAttributes( string, from, len, engine, si );
	openType->init(engine->glyphs(si), engine->glyphAttributes(si), si->num_glyphs,
		       engine->logClusters(si), len);

	openType->applyGSUBFeature(FT_MAKE_TAG( 'c', 'c', 'm', 'p' ));
	// Uniscribe also defines dlig for Hebrew, but we leave this out for now, as it's mostly
	// ligatures one does not want in modern Hebrew (as lam-alef ligatures).

	openType->applyGPOSFeatures();
	si->num_glyphs = 0;
	openType->appendTo(engine, si);

	return;
    }
#endif

    enum {
        Dagesh = 0x5bc,
        ShinDot = 0x5c1,
        SinDot = 0x5c2,
        Patah = 0x5b7,
        Qamats = 0x5b8, 
        Holam = 0x5b9,
        Rafe = 0x5bf
    };
    unsigned short chars[512];
    QChar *shapedChars = len > 256 ? (QChar *)::malloc(2*len * sizeof(QChar)) : (QChar *)chars;

    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );
    unsigned short *logClusters = engine->logClusters( si );
    
    const QChar *uc = string.unicode() + from;

    *shapedChars = *uc;
    logClusters[0] = 0;
    int slen = 1;
    int cluster_start = 0;
    for (int i = 1; i < len; ++i) {
        ushort base = shapedChars[slen-1].unicode();
        ushort shaped = 0;
        bool invalid = FALSE;
        if (uc[i].unicode() == Dagesh) {
            if (base >= 0x5d0
                && base <= 0x5ea
                && base != 0x5d7
                && base != 0x5dd 
                && base != 0x5df 
                && base != 0x5e2 
                && base != 0x5e5) {
                shaped = base - 0x5d0 + 0xfb30;
            } else if (base == 0xfb2a || base == 0xfb2b /* Shin with Shin or Sin dot */) {
                shaped = base + 2;
            } else {
                invalid = TRUE;
            }
        } else if (uc[i].unicode() == ShinDot) {
            if (base == 0x05e9)
                shaped = 0xfb2a;
            else if (base == 0xfb49)
                shaped = 0xfb2c;
            else
                invalid = TRUE;
        } else if (uc[i].unicode() == SinDot) {
            if (base == 0x05e9)
                shaped = 0xfb2b;
            else if (base == 0xfb49)
                shaped = 0xfb2d;
            else
                invalid = TRUE;
        } else if (uc[i].unicode() == Patah) {
            if (base == 0x5d0)
                shaped = 0xfb2e;
        } else if (uc[i].unicode() == Qamats) {
            if (base == 0x5d0)
                shaped = 0xfb2f;
        } else if (uc[i].unicode() == Holam) {
            if (base == 0x5d5)
                shaped = 0xfb4b;
        } else if (uc[i].unicode() == Rafe) {
            if (base == 0x5d1)
                shaped = 0xfb4c;
            else if (base == 0x5db)
                shaped = 0xfb4d;
            else if (base == 0x5e4)
                shaped = 0xfb4e;
        }

        if (invalid) {
            shapedChars[slen] = 0x25cc;
            glyphAttributes[slen].clusterStart = TRUE;
            glyphAttributes[slen].mark = FALSE;
            glyphAttributes[slen].combiningClass = 0;
            cluster_start = slen;
            ++slen;
        }
        if (shaped) {
            if (si->fontEngine->canRender((QChar *)&shaped, 1)) {
                shapedChars[slen-1] = QChar(shaped);
            } else
                shaped = 0;
        }
        if (!shaped) {
            shapedChars[slen] = uc[i];
            if (::category(uc[i]) != QChar::Mark_NonSpacing) {
                glyphAttributes[slen].clusterStart = TRUE;
                glyphAttributes[slen].mark = FALSE;
                glyphAttributes[slen].combiningClass = 0;
                cluster_start = slen;
            } else {
                glyphAttributes[slen].clusterStart = FALSE;
                glyphAttributes[slen].mark = TRUE;
                glyphAttributes[slen].combiningClass = ::combiningClass(uc[i]);
            }
            ++slen;
        }
        logClusters[i] = cluster_start;
        si->hasPositioning = TRUE;
    }
    
    convertToCMap(shapedChars, slen, engine, si);

    glyphAttributes = engine->glyphAttributes( si );
    advance_t *advances = engine->advances( si );
    {
        for (int i = 0; i < slen; ++i) {
            if (glyphAttributes[i].mark)
                advances[i] = 0;
        }
    }
    
    if ( !engine->widthOnly ) {
	q_heuristicPosition( engine, si );
    }

    if (len > 256)
        ::free(shapedChars);
}


/*
   Arabic shaping obeys a number of rules according to the joining classes (see Unicode book, section on
   arabic).

   Each unicode char has a joining class (right, dual (left&right), center (joincausing) or transparent).
   transparent joining is not encoded in QChar::joining(), but applies to all combining marks and format marks.

   Right join-causing: dual + center
   Left join-causing: dual + right + center

   Rules are as follows (for a string already in visual order, as we have it here):

   R1 Transparent characters do not affect joining behaviour.
   R2 A right joining character, that has a right join-causing char on the right will get form XRight
   (R3 A left joining character, that has a left join-causing char on the left will get form XLeft)
   Note: the above rule is meaningless, as there are no pure left joining characters defined in Unicode
   R4 A dual joining character, that has a left join-causing char on the left and a right join-causing char on
	     the right will get form XMedial
   R5  A dual joining character, that has a right join causing char on the right, and no left join causing char on the left
	 will get form XRight
   R6 A dual joining character, that has a  left join causing char on the left, and no right join causing char on the right
	 will get form XLeft
   R7 Otherwise the character will get form XIsolated

   Additionally we have to do the minimal ligature support for lam-alef ligatures:

   L1 Transparent characters do not affect ligature behaviour.
   L2 Any sequence of Alef(XRight) + Lam(XMedial) will form the ligature Alef.Lam(XLeft)
   L3 Any sequence of Alef(XRight) + Lam(XLeft) will form the ligature Alef.Lam(XIsolated)

   The two functions defined in this class do shaping in visual and logical order. For logical order just replace right with
   previous and left with next in the above rules ;-)
*/

/*
  Two small helper functions for arabic shaping. They get the next shape causing character on either
  side of the char in question. Implements rule R1.

  leftChar() returns true if the char to the left is a left join-causing char
  rightChar() returns true if the char to the right is a right join-causing char
*/


enum Shape {
    XIsolated,
    XFinal,
    XInitial,
    XMedial
};

/*
  Two small helper functions for arabic shaping. They get the next shape causing character on either
  side of the char in question. Implements rule R1.

  leftChar() returns true if the char to the left is a left join-causing char
  rightChar() returns true if the char to the right is a right join-causing char
*/
static inline const QChar *prevChar( const QString &str, int pos )
{
    //qDebug("leftChar: pos=%d", pos);
    pos--;
    const QChar *ch = str.unicode() + pos;
    while( pos > -1 ) {
	if( ::category( *ch ) != QChar::Mark_NonSpacing )
	    return ch;
	pos--;
	ch--;
    }
    return &QChar::replacement;
}

static inline const QChar *nextChar( const QString &str, int pos)
{
    pos++;
    int len = str.length();
    const QChar *ch = str.unicode() + pos;
    while( pos < len ) {
	//qDebug("rightChar: %d isLetter=%d, joining=%d", pos, ch.isLetter(), ch.joining());
	if( ::category( *ch ) != QChar::Mark_NonSpacing )
	    return ch;
	// assume it's a transparent char, this might not be 100% correct
	pos++;
	ch++;
    }
    return &QChar::replacement;
}

/* and the same thing for logical ordering :)
 */
static inline bool prevLogicalCharJoins( const QString &str, int pos)
{
    return ( joining( *nextChar( str, pos ) ) != QChar::OtherJoining );
}

static inline bool nextLogicalCharJoins( const QString &str, int pos)
{
    QChar::Joining join = joining( *prevChar( str, pos ) );
    return ( join == QChar::Dual || join == QChar::Center );
}


static inline Shape glyphVariantLogical( const QString &str, int pos)
{
    QChar::Joining joining = ::joining( str.unicode()[pos] );
    //qDebug("checking %x, joining=%d", str[pos].unicode(), joining);
    switch ( joining ) {
	case QChar::OtherJoining:
	case QChar::Center:
	    // these don't change shape
	    return XIsolated;
	case QChar::Right:
	    // only rule R2 applies
	    return ( nextLogicalCharJoins( str, pos ) ) ? XFinal : XIsolated;
	case QChar::Dual:
	    bool right = nextLogicalCharJoins( str, pos );
	    bool left = prevLogicalCharJoins( str, pos );
	    //qDebug("dual: right=%d, left=%d", right, left);
	    return ( right ) ? ( left ? XMedial : XFinal ) : ( left ? XInitial : XIsolated );
    }
    return XIsolated;
}


// The unicode to unicode shaping codec.
// does only presentation forms B at the moment, but that should be enough for
// simple display
static const ushort arabicUnicodeMapping[256][2] = {
    // base of shaped forms, and number-1 of them ( 0 for non shaping,
    // 1 for right binding and 3 for dual binding

    // These are just the glyphs available in Unicode,
    // some characters are in R class, but have no glyphs in Unicode.

    { 0x0600, 0 }, // 0x0600
    { 0x0601, 0 }, // 0x0601
    { 0x0602, 0 }, // 0x0602
    { 0x0603, 0 }, // 0x0603
    { 0x0604, 0 }, // 0x0604
    { 0x0605, 0 }, // 0x0605
    { 0x0606, 0 }, // 0x0606
    { 0x0607, 0 }, // 0x0607
    { 0x0608, 0 }, // 0x0608
    { 0x0609, 0 }, // 0x0609
    { 0x060A, 0 }, // 0x060A
    { 0x060B, 0 }, // 0x060B
    { 0x060C, 0 }, // 0x060C
    { 0x060D, 0 }, // 0x060D
    { 0x060E, 0 }, // 0x060E
    { 0x060F, 0 }, // 0x060F

    { 0x0610, 0 }, // 0x0610
    { 0x0611, 0 }, // 0x0611
    { 0x0612, 0 }, // 0x0612
    { 0x0613, 0 }, // 0x0613
    { 0x0614, 0 }, // 0x0614
    { 0x0615, 0 }, // 0x0615
    { 0x0616, 0 }, // 0x0616
    { 0x0617, 0 }, // 0x0617
    { 0x0618, 0 }, // 0x0618
    { 0x0619, 0 }, // 0x0619
    { 0x061A, 0 }, // 0x061A
    { 0x061B, 0 }, // 0x061B
    { 0x061C, 0 }, // 0x061C
    { 0x061D, 0 }, // 0x061D
    { 0x061E, 0 }, // 0x061E
    { 0x061F, 0 }, // 0x061F

    { 0x0620, 0 }, // 0x0620
    { 0xFE80, 0 }, // 0x0621            HAMZA
    { 0xFE81, 1 }, // 0x0622    R       ALEF WITH MADDA ABOVE
    { 0xFE83, 1 }, // 0x0623    R       ALEF WITH HAMZA ABOVE
    { 0xFE85, 1 }, // 0x0624    R       WAW WITH HAMZA ABOVE
    { 0xFE87, 1 }, // 0x0625    R       ALEF WITH HAMZA BELOW
    { 0xFE89, 3 }, // 0x0626    D       YEH WITH HAMZA ABOVE
    { 0xFE8D, 1 }, // 0x0627    R       ALEF
    { 0xFE8F, 3 }, // 0x0628    D       BEH
    { 0xFE93, 1 }, // 0x0629    R       TEH MARBUTA
    { 0xFE95, 3 }, // 0x062A    D       TEH
    { 0xFE99, 3 }, // 0x062B    D       THEH
    { 0xFE9D, 3 }, // 0x062C    D       JEEM
    { 0xFEA1, 3 }, // 0x062D    D       HAH
    { 0xFEA5, 3 }, // 0x062E    D       KHAH
    { 0xFEA9, 1 }, // 0x062F    R       DAL

    { 0xFEAB, 1 }, // 0x0630    R       THAL
    { 0xFEAD, 1 }, // 0x0631    R       REH
    { 0xFEAF, 1 }, // 0x0632    R       ZAIN
    { 0xFEB1, 3 }, // 0x0633    D       SEEN
    { 0xFEB5, 3 }, // 0x0634    D       SHEEN
    { 0xFEB9, 3 }, // 0x0635    D       SAD
    { 0xFEBD, 3 }, // 0x0636    D       DAD
    { 0xFEC1, 3 }, // 0x0637    D       TAH
    { 0xFEC5, 3 }, // 0x0638    D       ZAH
    { 0xFEC9, 3 }, // 0x0639    D       AIN
    { 0xFECD, 3 }, // 0x063A    D       GHAIN
    { 0x063B, 0 }, // 0x063B
    { 0x063C, 0 }, // 0x063C
    { 0x063D, 0 }, // 0x063D
    { 0x063E, 0 }, // 0x063E
    { 0x063F, 0 }, // 0x063F

    { 0x0640, 0 }, // 0x0640    C       TATWEEL // ### Join Causing, only one glyph
    { 0xFED1, 3 }, // 0x0641    D       FEH
    { 0xFED5, 3 }, // 0x0642    D       QAF
    { 0xFED9, 3 }, // 0x0643    D       KAF
    { 0xFEDD, 3 }, // 0x0644    D       LAM
    { 0xFEE1, 3 }, // 0x0645    D       MEEM
    { 0xFEE5, 3 }, // 0x0646    D       NOON
    { 0xFEE9, 3 }, // 0x0647    D       HEH
    { 0xFEED, 1 }, // 0x0648    R       WAW
    { 0x0649, 3 }, // 0x0649            ALEF MAKSURA // ### Dual, glyphs not consecutive, handle in code.
    { 0xFEF1, 3 }, // 0x064A    D       YEH
    { 0x064B, 0 }, // 0x064B
    { 0x064C, 0 }, // 0x064C
    { 0x064D, 0 }, // 0x064D
    { 0x064E, 0 }, // 0x064E
    { 0x064F, 0 }, // 0x064F

    { 0x0650, 0 }, // 0x0650
    { 0x0651, 0 }, // 0x0651
    { 0x0652, 0 }, // 0x0652
    { 0x0653, 0 }, // 0x0653
    { 0x0654, 0 }, // 0x0654
    { 0x0655, 0 }, // 0x0655
    { 0x0656, 0 }, // 0x0656
    { 0x0657, 0 }, // 0x0657
    { 0x0658, 0 }, // 0x0658
    { 0x0659, 0 }, // 0x0659
    { 0x065A, 0 }, // 0x065A
    { 0x065B, 0 }, // 0x065B
    { 0x065C, 0 }, // 0x065C
    { 0x065D, 0 }, // 0x065D
    { 0x065E, 0 }, // 0x065E
    { 0x065F, 0 }, // 0x065F

    { 0x0660, 0 }, // 0x0660
    { 0x0661, 0 }, // 0x0661
    { 0x0662, 0 }, // 0x0662
    { 0x0663, 0 }, // 0x0663
    { 0x0664, 0 }, // 0x0664
    { 0x0665, 0 }, // 0x0665
    { 0x0666, 0 }, // 0x0666
    { 0x0667, 0 }, // 0x0667
    { 0x0668, 0 }, // 0x0668
    { 0x0669, 0 }, // 0x0669
    { 0x066A, 0 }, // 0x066A
    { 0x066B, 0 }, // 0x066B
    { 0x066C, 0 }, // 0x066C
    { 0x066D, 0 }, // 0x066D
    { 0x066E, 0 }, // 0x066E
    { 0x066F, 0 }, // 0x066F

    { 0x0670, 0 }, // 0x0670
    { 0xFB50, 1 }, // 0x0671    R       ALEF WASLA
    { 0x0672, 0 }, // 0x0672
    { 0x0673, 0 }, // 0x0673
    { 0x0674, 0 }, // 0x0674
    { 0x0675, 0 }, // 0x0675
    { 0x0676, 0 }, // 0x0676
    { 0x0677, 0 }, // 0x0677
    { 0x0678, 0 }, // 0x0678
    { 0xFB66, 3 }, // 0x0679    D       TTEH
    { 0xFB5E, 3 }, // 0x067A    D       TTEHEH
    { 0xFB52, 3 }, // 0x067B    D       BEEH
    { 0x067C, 0 }, // 0x067C
    { 0x067D, 0 }, // 0x067D
    { 0xFB56, 3 }, // 0x067E    D       PEH
    { 0xFB62, 3 }, // 0x067F    D       TEHEH

    { 0xFB5A, 3 }, // 0x0680    D       BEHEH
    { 0x0681, 0 }, // 0x0681
    { 0x0682, 0 }, // 0x0682
    { 0xFB76, 3 }, // 0x0683    D       NYEH
    { 0xFB72, 3 }, // 0x0684    D       DYEH
    { 0x0685, 0 }, // 0x0685
    { 0xFB7A, 3 }, // 0x0686    D       TCHEH
    { 0xFB7E, 3 }, // 0x0687    D       TCHEHEH
    { 0xFB88, 1 }, // 0x0688    R       DDAL
    { 0x0689, 0 }, // 0x0689
    { 0x068A, 0 }, // 0x068A
    { 0x068B, 0 }, // 0x068B
    { 0xFB84, 1 }, // 0x068C    R       DAHAL
    { 0xFB82, 1 }, // 0x068D    R       DDAHAL
    { 0xFB86, 1 }, // 0x068E    R       DUL
    { 0x068F, 0 }, // 0x068F

    { 0x0690, 0 }, // 0x0690
    { 0xFB8C, 1 }, // 0x0691    R       RREH
    { 0x0692, 0 }, // 0x0692
    { 0x0693, 0 }, // 0x0693
    { 0x0694, 0 }, // 0x0694
    { 0x0695, 0 }, // 0x0695
    { 0x0696, 0 }, // 0x0696
    { 0x0697, 0 }, // 0x0697
    { 0xFB8A, 1 }, // 0x0698    R       JEH
    { 0x0699, 0 }, // 0x0699
    { 0x069A, 0 }, // 0x069A
    { 0x069B, 0 }, // 0x069B
    { 0x069C, 0 }, // 0x069C
    { 0x069D, 0 }, // 0x069D
    { 0x069E, 0 }, // 0x069E
    { 0x069F, 0 }, // 0x069F

    { 0x06A0, 0 }, // 0x06A0
    { 0x06A1, 0 }, // 0x06A1
    { 0x06A2, 0 }, // 0x06A2
    { 0x06A3, 0 }, // 0x06A3
    { 0xFB6A, 3 }, // 0x06A4    D       VEH
    { 0x06A5, 0 }, // 0x06A5
    { 0xFB6E, 3 }, // 0x06A6    D       PEHEH
    { 0x06A7, 0 }, // 0x06A7
    { 0x06A8, 0 }, // 0x06A8
    { 0xFB8E, 3 }, // 0x06A9    D       KEHEH
    { 0x06AA, 0 }, // 0x06AA
    { 0x06AB, 0 }, // 0x06AB
    { 0x06AC, 0 }, // 0x06AC
    { 0xFBD3, 3 }, // 0x06AD    D       NG
    { 0x06AE, 0 }, // 0x06AE
    { 0xFB92, 3 }, // 0x06AF    D       GAF

    { 0x06B0, 0 }, // 0x06B0
    { 0xFB9A, 3 }, // 0x06B1    D       NGOEH
    { 0x06B2, 0 }, // 0x06B2
    { 0xFB96, 3 }, // 0x06B3    D       GUEH
    { 0x06B4, 0 }, // 0x06B4
    { 0x06B5, 0 }, // 0x06B5
    { 0x06B6, 0 }, // 0x06B6
    { 0x06B7, 0 }, // 0x06B7
    { 0x06B8, 0 }, // 0x06B8
    { 0x06B9, 0 }, // 0x06B9
    { 0xfb9e, 1 }, // 0x06BA    R       NOON GHUNNA
    { 0xFBA0, 3 }, // 0x06BB    D       RNOON
    { 0x06BC, 0 }, // 0x06BC
    { 0x06BD, 0 }, // 0x06BD
    { 0xFBAA, 3 }, // 0x06BE    D       HEH DOACHASHMEE
    { 0x06BF, 0 }, // 0x06BF

    { 0xFBA4, 1 }, // 0x06C0    R       HEH WITH YEH ABOVE
    { 0xFBA6, 3 }, // 0x06C1    D       HEH GOAL
    { 0x06C2, 0 }, // 0x06C2
    { 0x06C3, 0 }, // 0x06C3
    { 0x06C4, 0 }, // 0x06C4
    { 0xFBE0, 1 }, // 0x06C5    R       KIRGHIZ OE
    { 0xFBD9, 1 }, // 0x06C6    R       OE
    { 0xFBD7, 1 }, // 0x06C7    R       U
    { 0xFBDB, 1 }, // 0x06C8    R       YU
    { 0xFBE2, 1 }, // 0x06C9    R       KIRGHIZ YU
    { 0x06CA, 0 }, // 0x06CA
    { 0xFBDE, 1 }, // 0x06CB    R       VE
    { 0xFBFC, 3 }, // 0x06CC    D       FARSI YEH
    { 0x06CD, 0 }, // 0x06CD
    { 0x06CE, 0 }, // 0x06CE
    { 0x06CF, 0 }, // 0x06CF

    { 0xFBE4, 3 }, // 0x06D0    D       E
    { 0x06D1, 0 }, // 0x06D1
    { 0xFBAE, 1 }, // 0x06D2    R       YEH BARREE
    { 0xFBB0, 1 }, // 0x06D3    R       YEH BARREE WITH HAMZA ABOVE
    { 0x06D4, 0 }, // 0x06D4
    { 0x06D5, 0 }, // 0x06D5
    { 0x06D6, 0 }, // 0x06D6
    { 0x06D7, 0 }, // 0x06D7
    { 0x06D8, 0 }, // 0x06D8
    { 0x06D9, 0 }, // 0x06D9
    { 0x06DA, 0 }, // 0x06DA
    { 0x06DB, 0 }, // 0x06DB
    { 0x06DC, 0 }, // 0x06DC
    { 0x06DD, 0 }, // 0x06DD
    { 0x06DE, 0 }, // 0x06DE
    { 0x06DF, 0 }, // 0x06DF

    { 0x06E0, 0 }, // 0x06E0
    { 0x06E1, 0 }, // 0x06E1
    { 0x06E2, 0 }, // 0x06E2
    { 0x06E3, 0 }, // 0x06E3
    { 0x06E4, 0 }, // 0x06E4
    { 0x06E5, 0 }, // 0x06E5
    { 0x06E6, 0 }, // 0x06E6
    { 0x06E7, 0 }, // 0x06E7
    { 0x06E8, 0 }, // 0x06E8
    { 0x06E9, 0 }, // 0x06E9
    { 0x06EA, 0 }, // 0x06EA
    { 0x06EB, 0 }, // 0x06EB
    { 0x06EC, 0 }, // 0x06EC
    { 0x06ED, 0 }, // 0x06ED
    { 0x06EE, 0 }, // 0x06EE
    { 0x06EF, 0 }, // 0x06EF

    { 0x06F0, 0 }, // 0x06F0
    { 0x06F1, 0 }, // 0x06F1
    { 0x06F2, 0 }, // 0x06F2
    { 0x06F3, 0 }, // 0x06F3
    { 0x06F4, 0 }, // 0x06F4
    { 0x06F5, 0 }, // 0x06F5
    { 0x06F6, 0 }, // 0x06F6
    { 0x06F7, 0 }, // 0x06F7
    { 0x06F8, 0 }, // 0x06F8
    { 0x06F9, 0 }, // 0x06F9
    { 0x06FA, 0 }, // 0x06FA
    { 0x06FB, 0 }, // 0x06FB
    { 0x06FC, 0 }, // 0x06FC
    { 0x06FD, 0 }, // 0x06FD
    { 0x06FE, 0 }, // 0x06FE
    { 0x06FF, 0 }  // 0x06FF
};

// the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, this table does
static const ushort alefMaksura[4] = {0xFEEF, 0xFEF0, 0xFBE8, 0xFBE9};

// this is a bit tricky. Alef always binds to the right, so the second parameter descibing the shape
// of the lam can be either initial of medial. So initial maps to the isolated form of the ligature,
// medial to the final form
static const ushort arabicUnicodeLamAlefMapping[6][4] = {
    { 0xfffd, 0xfffd, 0xfef5, 0xfef6 }, // 0x622        R       Alef with Madda above
    { 0xfffd, 0xfffd, 0xfef7, 0xfef8 }, // 0x623        R       Alef with Hamza above
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x624        // Just to fill the table ;-)
    { 0xfffd, 0xfffd, 0xfef9, 0xfefa }, // 0x625        R       Alef with Hamza below
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x626        // Just to fill the table ;-)
    { 0xfffd, 0xfffd, 0xfefb, 0xfefc }  // 0x627        R       Alef
};

static inline int getShape( uchar cell, int shape )
{
    if (arabicUnicodeMapping[cell][1] < shape)
        return 0x600+cell;

#ifdef Q_OS_MAC
    // the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, handle this here
    uint ch = ( cell != 0x49 )
	      ? (shape ? arabicUnicodeMapping[cell][0] + shape : 0x600+cell)
	      : alefMaksura[shape] ;
#else
    // the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, handle this here
    uint ch = ( cell != 0x49 ) ? arabicUnicodeMapping[cell][0] + shape
	    		       : alefMaksura[shape] ;
#endif
    return ch;
}


static void shapedString(const QString& uc, int from, int len, QChar *shapeBuffer, int *shapedLength,
			 bool reverse, GlyphAttributes *attrs, unsigned short *logClusters )
{
    if( len < 0 ) {
	len = uc.length() - from;
    } else if( len == 0 ) {
	*shapedLength = 0;
	return;
    }

    const QChar *ch = uc.unicode() + from;
    QChar *data = shapeBuffer;
    int clusterStart = 0;

    for ( int i = 0; i < len; i++ ) {
	uchar r = ch->row();
	int gpos = data - shapeBuffer;

	if ( r != 0x06 ) {
	    if ( r == 0x20 ) {
		uchar c = ch->cell();
		if (c == 0x0c || c == 0x0d)
		    // remove ZWJ and ZWNJ
		    goto skip;
	    }
	    if ( reverse )
		*data = mirroredChar( *ch );
	    else
		*data = *ch;
	} else {
	    uchar c = ch->cell();
	    int pos = i + from;
	    int shape = glyphVariantLogical( uc, pos );
//   	    qDebug("mapping U+%x to shape %d glyph=0x%x", ch->unicode(), shape, getShape(c, shape));
	    // take care of lam-alef ligatures (lam right of alef)
	    ushort map;
	    switch ( c ) {
		case 0x44: { // lam
		    const QChar *pch = nextChar( uc, pos );
		    if ( pch->row() == 0x06 ) {
			switch ( pch->cell() ) {
			    case 0x22:
			    case 0x23:
			    case 0x25:
			    case 0x27:
// 				qDebug(" lam of lam-alef ligature");
				map = arabicUnicodeLamAlefMapping[pch->cell() - 0x22][shape];
				goto next;
			    default:
				break;
			}
		    }
		    break;
		}
		case 0x22: // alef with madda
		case 0x23: // alef with hamza above
		case 0x25: // alef with hamza below
		case 0x27: // alef
		    if ( prevChar( uc, pos )->unicode() == 0x0644 ) {
			// have a lam alef ligature
			//qDebug(" alef of lam-alef ligature");
			goto skip;
		    }
		default:
		    break;
	    }
	    map = getShape( c, shape );
	next:
	    *data = map;
	}
	// #### Qt 4.0: Fixme
	attrs[gpos].zeroWidth = FALSE;
	if ( ::category( *ch ) == QChar::Mark_NonSpacing ) {
	    attrs[gpos].mark = TRUE;
// 	    qDebug("glyph %d (char %d) is mark!", gpos, i );
	} else {
	    attrs[gpos].mark = FALSE;
	    clusterStart = data - shapeBuffer;
	}
	attrs[gpos].clusterStart = !attrs[gpos].mark;
	attrs[gpos].combiningClass = combiningClass( *ch );
	data++;
    skip:
	ch++;
	logClusters[i] = clusterStart;
    }
    *shapedLength = data - shapeBuffer;
}

#if defined( Q_WS_X11) && !defined( QT_NO_XFTFREETYPE )

static void arabicSyriacOpenTypeShape( int script, QOpenType *openType, const QString &string, int from,
				       int len, QTextEngine *engine, QScriptItem *si )
{
    convertToCMap( string.unicode() + from, len, engine, si );
    heuristicSetGlyphAttributes( string, from, len, engine, si );

    unsigned char gv[256];
    unsigned char *glyphVariant = gv;
    bool ap[256];
    bool *apply = ap;

    if ( si->num_glyphs > 255 ) {
	glyphVariant = (unsigned char *)malloc(si->num_glyphs*sizeof(unsigned char));
	apply = (bool *)malloc(si->num_glyphs*sizeof(bool));
    }

    for ( int i = 0; i < si->num_glyphs; i++ )
	glyphVariant[i] = glyphVariantLogical( string, from + i );

    glyph_t *glyphs = engine->glyphs(si);
    GlyphAttributes *attrs = engine->glyphAttributes(si);
    unsigned short *logClusters = engine->logClusters(si);
    unsigned short *uc = (unsigned short *)string.unicode() + from;

    // Hack to remove ZWJ and ZWNJ from rendered output.
    int j = 0;
    for ( int i = 0; i < si->num_glyphs; i++ ) {
	if (uc[i] == 0x200c || uc[i] == 0x200d)
	    continue;
	glyphs[j] = glyphs[i];
	attrs[j] = attrs[i];
	glyphVariant[j] = glyphVariant[i];
	logClusters[i] = logClusters[j];
	++j;
    }
    si->num_glyphs = j;

    openType->init(glyphs, attrs, si->num_glyphs, logClusters, si->num_glyphs);

    // call features in the order defined by http://www.microsoft.com/typography/otfntdev/arabicot/shaping.htm
    openType->applyGSUBFeature(FT_MAKE_TAG( 'c', 'c', 'm', 'p' ));

    if (script == QFont::Arabic) {
	const struct {
	    int tag;
	    int shape;
	} features[] = {
	    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), XIsolated },
	    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), XFinal },
	    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), XMedial },
	    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), XInitial }
	};
	for (int j = 0; j < 4; ++j) {
	    for ( int i = 0; i < si->num_glyphs; i++ )
		apply[i] = (glyphVariant[i] == features[j].shape);
	    openType->applyGSUBFeature(features[j].tag, apply);
	}
    } else {
	const struct {
	    int tag;
	    int shape;
	} features[] = {
	    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), XIsolated },
	    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), XFinal },
	    { FT_MAKE_TAG( 'f', 'i', 'n', '2' ), XFinal },
	    { FT_MAKE_TAG( 'f', 'i', 'n', '3' ), XFinal },
	    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), XMedial },
	    { FT_MAKE_TAG( 'm', 'e', 'd', '2' ), XMedial },
	    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), XInitial }
	};
	for (int j = 0; j < 7; ++j) {
	    for ( int i = 0; i < si->num_glyphs; i++ )
		apply[i] = (glyphVariant[i] == features[j].shape);
	    openType->applyGSUBFeature(features[j].tag, apply);
	}
    }
    const int commonFeatures[] = {
	// these features get applied to all glyphs and both scripts
	FT_MAKE_TAG( 'r', 'l', 'i', 'g' ),
	FT_MAKE_TAG( 'c', 'a', 'l', 't' ),
	FT_MAKE_TAG( 'l', 'i', 'g', 'a' ),
	FT_MAKE_TAG( 'd', 'l', 'i', 'g' )
    };
    for (int j = 0; j < 4; ++j)
	openType->applyGSUBFeature(commonFeatures[j]);

    if (script == QFont::Arabic) {
	const int features[] = {
	    FT_MAKE_TAG( 'c', 's', 'w', 'h' ),
	    // mset is used in old Win95 fonts that don't have a 'mark' positioning table.
	    FT_MAKE_TAG( 'm', 's', 'e', 't' )
	};
	for (int j = 0; j < 2; ++j)
	    openType->applyGSUBFeature(features[j]);
    }

    openType->applyGPOSFeatures();
    si->num_glyphs = 0;
    openType->appendTo(engine, si);

    if (glyphVariant != gv) {
	free(glyphVariant);
	free(apply);
    }
}

#endif

static void arabic_attributes( int /*script*/, const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    attributes += from;
    for ( int i = 0; i < len; i++ ) {
	QChar::Category cat = ::category( *uc );
	attributes->whiteSpace = (cat == QChar::Separator_Space) && (uc->unicode() != 0xa0);
	attributes->softBreak = FALSE;
	attributes->charStop = (cat != QChar::Mark_NonSpacing);
	attributes->wordStop = FALSE;
	attributes->invalid = FALSE;
	++uc;
	++attributes;
    }
}


// #### stil missing: identify invalid character combinations
static void arabic_shape( int /*script*/, const QString &string, int from, int len,
			  QTextEngine *engine, QScriptItem *si )
{
#if defined( Q_WS_X11) && !defined( QT_NO_XFTFREETYPE )
    QOpenType *openType = si->fontEngine->openType();

    if ( openType && openType->supportsScript( QFont::Arabic ) ) {
	arabicSyriacOpenTypeShape( QFont::Arabic, openType, string,  from,  len, engine, si );
	return;
    }
#endif

    const QString &text = string;

    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );
    unsigned short *logClusters = engine->logClusters( si );

    QChar *shapedChars = (QChar *)::malloc( len * sizeof( QChar ) );

    int slen;
    shapedString( text, from, len, shapedChars, &slen, (si->analysis.bidiLevel%2),
		  glyphAttributes, logClusters );

    convertToCMap( shapedChars, slen, engine, si );
    advance_t *advances = engine->advances(si);
    for (int i = 0; i < slen; ++i)
	if (glyphAttributes[i].mark)
	    advances[i] = 0;

    ::free( shapedChars );
    q_heuristicPosition( engine, si );
}

#ifdef Q_WS_X11
# include "qscriptengine_x11.cpp"
#elif defined( Q_WS_WIN )
# include "qscriptengine_win.cpp"
#elif defined( Q_WS_MAC )
# include "qscriptengine_mac.cpp"
#elif defined( Q_WS_QWS )
# include "qscriptengine_qws.cpp"
#endif
