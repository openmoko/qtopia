/****************************************************************************
** $Id: qt/qfontengine_x11.cpp   3.3.5   edited Aug 31 12:13 $
**
** ???
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
** licenses for Unix/X11 or for Qt/Embedded may use this file in accordance
** with the Qt Commercial License Agreement provided with the Software.
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

#include "qfontengine_p.h"

// #define FONTENGINE_DEBUG

#include <qwidget.h>
#include <qcstring.h>
#include <qtextcodec.h>

#include "qbitmap.h"
#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qimage.h"

#include "qt_x11_p.h"

#include "qfont.h"
#include "qtextengine_p.h"

#include <private/qunicodetables_p.h>

#include <limits.h>

// defined in qfontdatbase_x11.cpp
extern int qt_mib_for_xlfd_encoding( const char *encoding );
extern int qt_xlfd_encoding_id( const char *encoding );

extern void qt_draw_transformed_rect( QPainter *p, int x, int y, int w, int h, bool fill );

static void drawLines( QPainter *p, QFontEngine *fe, int baseline, int x1, int w, int textFlags )
{
    int lw = fe->lineThickness();
    if ( textFlags & Qt::Underline ) {
    	int pos = fe->underlinePosition();
	qt_draw_transformed_rect( p, x1, baseline+pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::Overline ) {
	int pos = fe->ascent()+1;
	if ( !pos ) pos = 1;
	qt_draw_transformed_rect( p, x1, baseline-pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::StrikeOut ) {
	int pos = fe->ascent()/3;
	if ( !pos ) pos = 1;
	qt_draw_transformed_rect( p, x1, baseline-pos, w, lw, TRUE );
    }
}


inline static void qSafeXDestroyImage( XImage *x )
{
    if ( x->data ) {
	free( x->data );
	x->data = 0;
    }
    XDestroyImage( x );
}

extern bool qt_xForm_helper( const QWMatrix &trueMat, int xoffset,
                             int type, int depth,
                             uchar *dptr, int dbpl, int p_inc, int dHeight,
                             uchar *sptr, int sbpl, int sWidth, int sHeight
    );

static QBitmap transform(Display *dpy, const QBitmap &source, int xoff,  int yoff, int w, int h, const QWMatrix &matrix)
{
    int ws = source.width();
    int hs = source.height();

    bool invertible;
    QWMatrix mat = matrix.invert( &invertible );		// invert matrix

    if (!invertible )
	return QBitmap();
    mat.translate(xoff, yoff);

    XImage *xi = XGetImage(dpy, source.handle(), 0, 0, ws, hs, AllPlanes, XYPixmap);

    if ( !xi )
        return QBitmap();

    int sbpl = xi->bytes_per_line;
    uchar *sptr = (uchar *)xi->data;

    int dbpl = (w+7)/8;
    int dbytes = dbpl*h;

    uchar *dptr = (uchar *)malloc( dbytes );	// create buffer for bits
    memset( dptr, 0, dbytes );

    int type = xi->bitmap_bit_order == MSBFirst ? QT_XFORM_TYPE_MSBFIRST : QT_XFORM_TYPE_LSBFIRST;
    int	xbpl, p_inc;
    xbpl  = (w+7)/8;
    p_inc = dbpl - xbpl;

    bool ok = qt_xForm_helper( mat, xi->xoffset, type, 1, dptr, xbpl, p_inc, h, sptr, sbpl, ws, hs );
    qSafeXDestroyImage(xi);
    QBitmap bm;
    if (ok) {
        bm = QBitmap( w, h, dptr, QImage::systemBitOrder() != QImage::BigEndian );
    } else {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFontEngineXft::tranform: xform failed");
#endif
    }

    free( dptr );
    return bm;
}


static void drawScaled(int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags,
    Display *dpy, GC gc, QPaintDevice *pdev, QFontEngine *fe, const QWMatrix &xmat)
{
    // font doesn't support transformations, need to do it by hand
    int w = si->width, h = si->ascent + si->descent + 1;
    if ( w == 0 || h == 0 )
        return;

    w += h; // add some pixels to width because of italic correction
    QBitmap bm( w, h, TRUE );     // create bitmap 
    QPainter paint;
    paint.begin( &bm );             // draw text in bitmap
    fe->draw( &paint, 0, si->ascent, engine, si, textFlags );
    paint.end();

    QRect pdevRect;
    if (pdev->devType() == QInternal::Widget)
        pdevRect = ((QWidget *)pdev)->rect();
    else if (pdev->devType() == QInternal::Pixmap)
        pdevRect = ((QPixmap *)pdev)->rect();
    else
        return;


    QRect br = xmat.mapRect(QRect(x, y - si->ascent, w, h));
    QRect br2 = br & pdevRect;
    if (br2.width() <= 0 || br2.height() <= 0)
        return;
    QWMatrix mat = QPixmap::trueMatrix( xmat, w, h );
    QBitmap wx_bm = ::transform(dpy, bm, br2.x() - br.x(), br2.y() - br.y(), br2.width(), br2.height(), mat);
    if ( wx_bm.isNull() )
        return;

    x = br2.x();
    y = br2.y();

    Qt::HANDLE hd = pdev->handle();
    XSetFillStyle( dpy, gc, FillStippled );
    XSetStipple( dpy, gc, wx_bm.handle() );
    XSetTSOrigin( dpy, gc, x, y );
    XFillRectangle( dpy, hd, gc, x, y, wx_bm.width(), wx_bm.height() );
    XSetTSOrigin( dpy, gc, 0, 0 );
    XSetFillStyle( dpy, gc, FillSolid );
}


QFontEngine::~QFontEngine()
{
}

int QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 ) lw = 2;
    if ( lw == 0 ) lw = 1;

    return lw;
}

int QFontEngine::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}

// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------


QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{
    cache_cost = sizeof( QFontEngineBox );
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    memset( glyphs, 0, len * sizeof( glyph_t ) );
    *nglyphs = len;

    if ( advances ) {
	for ( int i = 0; i < len; i++ )
	    *(advances++) = _size;
    }
    return NoError;
}

void QFontEngineBox::draw( QPainter *p, int x, int y, const QTextEngine *, const QScriptItem *si, int textFlags )
{
    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif


    if ( p->txop > QPainter::TxTranslate ) {
	int xp = x;
	int yp = _size + 2;
	int s = _size - 3;
	for (int k = 0; k < si->num_glyphs; k++) {
	    qt_draw_transformed_rect( p, xp, yp, s, s, FALSE );
	    xp += _size;
	}
    } else {
	if ( p->txop == QPainter::TxTranslate )
	    p->map( x, y, &x, &y );

	XRectangle rects[64];

	int gl = 0;
	while (gl < si->num_glyphs) {
	    int toDraw = QMIN(64, si->num_glyphs-gl);
	    int adv = toDraw*_size;
	    if (x + adv < SHRT_MAX && x > SHRT_MIN) {
		for (int k = 0; k < toDraw; k++) {
		    rects[k].x = x + (k * _size);
		    rects[k].y = y - _size + 2;
		    rects[k].width = rects[k].height = _size - 3;
		}
		XDrawRectangles(dpy, hd, gc, rects, toDraw);
	    }
	    gl += toDraw;
	    x += adv;
	}
    }

    if ( textFlags != 0 )
	drawLines( p, this, y, x, si->num_glyphs*_size, textFlags );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < numGlyphs; i++ ) {
	glyph_metrics_t ci = boundingBox( glyphs[i] );
	x += offsets[i].x;
	y += offsets[i].y;
	p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offset=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y );
	x += ci.xoff;
	y += ci.yoff;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineBox::boundingBox( const glyph_t *, const advance_t *, const qoffset_t *, int numGlyphs )
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

glyph_metrics_t QFontEngineBox::boundingBox( glyph_t )
{
    return glyph_metrics_t( 0, _size, _size, _size, _size, 0 );
}



int QFontEngineBox::ascent() const
{
    return _size;
}

int QFontEngineBox::descent() const
{
    return 0;
}

int QFontEngineBox::leading() const
{
    int l = qRound( _size * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineBox::maxCharWidth() const
{
    return _size;
}

int QFontEngineBox::cmap() const
{
    return -1;
}

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender( const QChar *, int )
{
    return TRUE;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}




// ------------------------------------------------------------------
// Xlfd cont engine
// ------------------------------------------------------------------

static inline XCharStruct *charStruct( XFontStruct *xfs, uint ch )
{
    XCharStruct *xcs = 0;
    unsigned char r = ch>>8;
    unsigned char c = ch&0xff;
    if ( r >= xfs->min_byte1 &&
	 r <= xfs->max_byte1 &&
	 c >= xfs->min_char_or_byte2 &&
	 c <= xfs->max_char_or_byte2) {
	if ( !xfs->per_char )
	    xcs = &(xfs->min_bounds);
	else {
	    xcs = xfs->per_char + ((r - xfs->min_byte1) *
				   (xfs->max_char_or_byte2 -
				    xfs->min_char_or_byte2 + 1)) +
		  (c - xfs->min_char_or_byte2);
	    if (xcs->width == 0 && xcs->ascent == 0 &&  xcs->descent == 0)
		xcs = 0;
	}
    }
    return xcs;
}

QFontEngineXLFD::QFontEngineXLFD( XFontStruct *fs, const char *name, int mib )
    : _fs( fs ), _name( name ), _codec( 0 ), _scale( 1. ), _cmap( mib )
{
    if ( _cmap ) _codec = QTextCodec::codecForMib( _cmap );

    cache_cost = (((fs->max_byte1 - fs->min_byte1) *
		   (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)) +
		  fs->max_char_or_byte2 - fs->min_char_or_byte2);
    cache_cost = ((fs->max_bounds.ascent + fs->max_bounds.descent) *
		  (fs->max_bounds.width * cache_cost / 8));
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

#if 1
    // Server side transformations do not seem to work correctly for
    // all types of fonts (for example, it works for bdf/pcf fonts,
    // but not for ttf).  It also seems to be extermely server
    // dependent.  The best thing is to just disable server side
    // transformations until either server support matures or we
    // figure out a better way to do it.
    xlfd_transformations = XlfdTrUnsupported;
#else
    xlfd_transformations = XlfdTrUnknown;

    // Hummingbird's Exceed X server will substitute 'fixed' for any
    // known fonts, and it doesn't seem to support transformations, so
    // we should never try to use xlfd transformations with it
    if (strstr(ServerVendor(QPaintDevice::x11AppDisplay()), "Hummingbird"))
	xlfd_transformations = XlfdTrUnsupported;
#endif
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont( QPaintDevice::x11AppDisplay(), _fs );
    _fs = 0;
    TransformedFont *trf = transformed_fonts;
    while ( trf ) {
	XUnloadFont( QPaintDevice::x11AppDisplay(), trf->xlfd_font );
	TransformedFont *tmp = trf;
	trf = trf->next;
	delete tmp;
    }
}

QFontEngine::Error QFontEngineXLFD::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( _codec ) {
	bool haveNbsp = FALSE;
	for ( int i = 0; i < len; i++ )
	    if ( str[i].unicode() == 0xa0 ) {
		haveNbsp = TRUE;
		break;
	    }

	QChar *chars = (QChar *)str;
	if ( haveNbsp || mirrored ) {
	    chars = (QChar *)malloc( len*sizeof(QChar) );
	    for ( int i = 0; i < len; i++ )
		chars[i] = (str[i].unicode() == 0xa0 ? 0x20 :
			    (mirrored ? ::mirroredChar(str[i]).unicode() : str[i].unicode()));
	}
	_codec->fromUnicodeInternal( chars, glyphs, len );
	if (chars != str)
	    free( chars );
    } else {
	glyph_t *g = glyphs + len;
	const QChar *c = str + len;
	if ( mirrored ) {
	    while ( c != str )
		*(--g) = (--c)->unicode() == 0xa0 ? 0x20 : ::mirroredChar(*c).unicode();
	} else {
	    while ( c != str )
		*(--g) = (--c)->unicode() == 0xa0 ? 0x20 : c->unicode();
	}
    }
    *nglyphs = len;

    if ( advances ) {
	glyph_t *g = glyphs + len;
	advance_t *a = advances + len;
	XCharStruct *xcs;
	// inlined for better perfomance
	if ( !_fs->per_char ) {
	    xcs = &_fs->min_bounds;
	    while ( a != advances )
		*(--a) = xcs->width;
	}
	else if ( !_fs->max_byte1 ) {
	    XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
	    while ( g-- != glyphs ) {
		unsigned int gl = *g;
		xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
		      base + gl : 0;
		*(--a) = (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) ? _fs->ascent : xcs->width;
	    }
	}
	else {
	    while ( g != glyphs ) {
		xcs = charStruct( _fs, *(--g) );
		*(--a) = (xcs ? xcs->width : _fs->ascent);
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
    return NoError;
}

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static bool x_font_load_error = FALSE;
static int x_font_errorhandler(Display *, XErrorEvent *)
{
    x_font_load_error = TRUE;
    return 0;
}

#if defined(Q_C_CALLBACKS)
}
#endif


void QFontEngineXLFD::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs )
	return;

//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, si->num_glyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

    bool transform = FALSE;
    int xorig = x;
    int yorig = y;

    Qt::HANDLE font_id = _fs->fid;
    if ( p->txop > QPainter::TxTranslate || _scale < 0.9999 || _scale > 1.0001  ) {
	bool degenerate = QABS( p->m11()*p->m22() - p->m12()*p->m21() ) < 0.01;
	if ( !degenerate && xlfd_transformations != XlfdTrUnsupported ) {
	    // need a transformed font from the server
	    QCString xlfd_transformed = _name;
	    int field = 0;
	    char *data = xlfd_transformed.data();
	    int pos = 0;
	    while ( field < 7 ) {
		if ( data[pos] == '-' )
		    field++;
		pos++;
	    }
	    int endPos = pos;
	    while ( data[endPos] != '-' )
		endPos++;
	    float size = xlfd_transformed.mid( pos, endPos-pos ).toInt();
	    float mat[4];
	    mat[0] = p->m11()*size*_scale;
	    mat[1] = -p->m12()*size*_scale;
	    mat[2] = -p->m21()*size*_scale;
	    mat[3] = p->m22()*size*_scale;

	    // check if we have it cached
	    TransformedFont *trf = transformed_fonts;
	    TransformedFont *prev = 0;
	    int i = 0;
	    while ( trf ) {
		if ( trf->xx == mat[0] &&
		     trf->xy == mat[1] &&
		     trf->yx == mat[2] &&
		     trf->yy == mat[3] )
		    break;
		TransformedFont *tmp = trf;
		trf = trf->next;
		if (i > 10) {
		    XUnloadFont( QPaintDevice::x11AppDisplay(), tmp->xlfd_font );
		    delete tmp;
		    prev->next = trf;
		} else {
		    prev = tmp;
		}
		++i;
	    }
	    if ( trf ) {
		if ( prev ) {
		    // move to beginning of list
		    prev->next = trf->next;
		    trf->next = transformed_fonts;
		    transformed_fonts = trf;
		}
		font_id = trf->xlfd_font;
	    } else {
		QCString matrix="[";
		for ( int i = 0; i < 4; i++ ) {
		    float f = mat[i];
		    if ( f < 0 ) {
			matrix += '~';
			f = -f;
		    }
		    matrix += QString::number( f, 'f', 5 ).latin1();
		    matrix += ' ';
		}
		matrix += ']';
		//qDebug("m: %2.2f %2.2f %2.2f %2.2f, matrix=%s", p->m11(), p->m12(), p->m21(), p->m22(), matrix.data());
		xlfd_transformed.replace( pos, endPos-pos, matrix );

		x_font_load_error = FALSE;
		XErrorHandler old_handler = XSetErrorHandler( x_font_errorhandler );
		font_id = XLoadFont( dpy, xlfd_transformed.data() );
		XSync( dpy, FALSE );
		XSetErrorHandler( old_handler );
		if ( x_font_load_error ) {
		    //qDebug( "couldn't load transformed font" );
		    font_id = _fs->fid;
		    xlfd_transformations = XlfdTrUnsupported;
		} else {
		    TransformedFont *trf = new TransformedFont;
		    trf->xx = mat[0];
		    trf->xy = mat[1];
		    trf->yx = mat[2];
		    trf->yy = mat[3];
		    trf->xlfd_font = font_id;
		    trf->next = transformed_fonts;
		    transformed_fonts = trf;
		}
	    }
	}
	if ( degenerate || xlfd_transformations == XlfdTrUnsupported ) {
	    // XServer or font don't support server side transformations, need to do it by hand
            drawScaled(x, y, engine, si, textFlags, dpy, p->gc, p->device(), this, p->xmat);
	    return;
        }
	transform = TRUE;
    } else if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    XSetFont(dpy, gc, font_id);

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, si->num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    XChar2b ch[256];
    XChar2b *chars = ch;
    if ( si->num_glyphs > 255 )
	chars = (XChar2b *)malloc( si->num_glyphs*sizeof(XChar2b) );

    for (int i = 0; i < si->num_glyphs; i++) {
	chars[i].byte1 = glyphs[i] >> 8;
	chars[i].byte2 = glyphs[i] & 0xff;
    }

    int xpos = x;

    if ( si->analysis.bidiLevel % 2 ) {
	int i = si->num_glyphs;
	while( i-- ) {
	    advance_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
	    glyph_metrics_t gi = boundingBox( glyphs[i] );
	    int xp = x-offsets[i].x-gi.xoff;
	    int yp = y+offsets[i].y-gi.yoff;
	    if ( transform )
		p->map( xp, yp, &xp, &yp );
	    if (xp < SHRT_MAX && xp > SHRT_MIN)
		XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
	}
    } else {
	if ( transform || si->hasPositioning ) {
	    int i = 0;
	    while( i < si->num_glyphs ) {
		int xp = x+offsets[i].x;
		int yp = y+offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
		if (xp < SHRT_MAX && xp > SHRT_MIN)
		    XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
		advance_t adv = advances[i];
		// 	    qDebug("advance = %d/%d", adv.x, adv.y );
		x += adv;
		i++;
	    }
	} else {
	    // we can take a shortcut
	    int gl = 0;
	    while (gl < si->num_glyphs) {
		int toDraw = QMIN(64, si->num_glyphs-gl);
		int adv = 0;
		for (int i = gl; i < gl+toDraw; ++i)
		    adv += advances[i];
		if (x + adv < SHRT_MAX && x > SHRT_MIN)
		    XDrawString16(dpy, hd, gc, x, y, chars+gl, toDraw);
		gl += toDraw;
		x += adv;
	    }
	}
    }

    if ( chars != ch )
	free( chars );

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, x-xpos, textFlags );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < si->num_glyphs; i++ ) {
	glyph_metrics_t ci = boundingBox( glyphs[i] );
	p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y,
	       advances[i].x, advances[i].y);
	x += advances[i].x;
	y += advances[i].y;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineXLFD::boundingBox( const glyph_t *glyphs, const advance_t *advances, const qoffset_t *offsets, int numGlyphs )
{
    int i;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    for (i = 0; i < numGlyphs; i++) {
	XCharStruct *xcs = charStruct( _fs, glyphs[i] );
	if (xcs) {
	    int x = overall.xoff + offsets[i].x - xcs->lbearing;
	    int y = overall.yoff + offsets[i].y - xcs->ascent;
	    overall.x = QMIN( overall.x, x );
	    overall.y = QMIN( overall.y, y );
	    xmax = QMAX( xmax, overall.xoff + offsets[i].x + xcs->rbearing );
	    ymax = QMAX( ymax, y + xcs->ascent + xcs->descent );
	    overall.xoff += qRound(advances[i]/_scale);
	} else {
	    int size = _fs->ascent;
	    overall.x = QMIN(overall.x, overall.xoff );
	    overall.y = QMIN(overall.y, overall.yoff - size );
	    ymax = QMAX( ymax, overall.yoff );
	    overall.xoff += size;
	    xmax = QMAX( xmax, overall.xoff );
	}
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if ( _scale != 1. ) {
	overall.x = qRound(overall.x * _scale);
	overall.y = qRound(overall.y * _scale);
	overall.height = qRound(overall.height * _scale);
	overall.width = qRound(overall.width * _scale);
	overall.xoff = qRound(overall.xoff * _scale);
	overall.yoff = qRound(overall.yoff * _scale);
    }
    return overall;
}

glyph_metrics_t QFontEngineXLFD::boundingBox( glyph_t glyph )
{
    glyph_metrics_t gm;
    // ### scale missing!
    XCharStruct *xcs = charStruct( _fs, glyph );
    if (xcs) {
	gm = glyph_metrics_t( xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent, xcs->width, 0 );
    } else {
	int size = _fs->ascent;
	gm = glyph_metrics_t( 0, size, size, size, size, 0 );
    }
    if ( _scale != 1. ) {
	gm.x = qRound(gm.x * _scale);
	gm.y = qRound(gm.y * _scale);
	gm.height = qRound(gm.height * _scale);
	gm.width = qRound(gm.width * _scale);
	gm.xoff = qRound(gm.xoff * _scale);
	gm.yoff = qRound(gm.yoff * _scale);
    }
    return gm;
}


int QFontEngineXLFD::ascent() const
{
    return qRound(_fs->ascent*_scale);
}

int QFontEngineXLFD::descent() const
{
    return qRound((_fs->descent-1)*_scale);
}

int QFontEngineXLFD::leading() const
{
    int l = qRound((QMIN(_fs->ascent, _fs->max_bounds.ascent)
		    + QMIN(_fs->descent, _fs->max_bounds.descent)) * _scale * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineXLFD::maxCharWidth() const
{
    return qRound(_fs->max_bounds.width*_scale);
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
	     (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
	    f->max_char_or_byte2 - f->min_char_or_byte2);
}

int QFontEngineXLFD::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN ) {
	if ( _fs->per_char ) {
	    XCharStruct *cs = _fs->per_char;
	    int nc = maxIndex(_fs) + 1;
	    int mx = cs->lbearing;

	    for (int c = 1; c < nc; c++) {
		// ignore the bearings for characters whose ink is
		// completely outside the normal bounding box
		if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
		    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
		    continue;

		int nmx = cs[c].lbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->lbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->lbearing = _fs->min_bounds.lbearing;
    }
    return qRound (lbearing*_scale);
}

int QFontEngineXLFD::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
	if ( _fs->per_char ) {
	    XCharStruct *cs = _fs->per_char;
	    int nc = maxIndex(_fs) + 1;
	    int mx = cs->rbearing;

	    for (int c = 1; c < nc; c++) {
		// ignore the bearings for characters whose ink is
		// completely outside the normal bounding box
		if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
		    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
		    continue;

		int nmx = cs[c].rbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->rbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->rbearing = _fs->min_bounds.rbearing;
    }
    return qRound (rbearing*_scale);
}

int QFontEngineXLFD::cmap() const
{
    return _cmap;
}

const char *QFontEngineXLFD::name() const
{
    return _name;
}

bool QFontEngineXLFD::canRender( const QChar *string, int len )
{
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs, FALSE ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs, FALSE );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !g[i] || !charStruct( _fs, g[i] ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( g != glyphs )
	free( g );

    return allExist;
}


void QFontEngineXLFD::setScale( double scale )
{
    _scale = scale;
}


QFontEngine::Type QFontEngineXLFD::type() const
{
    return XLFD;
}


// ------------------------------------------------------------------
// LatinXLFD engine
// ------------------------------------------------------------------

static const int engine_array_inc = 4;

QFontEngineLatinXLFD::QFontEngineLatinXLFD( XFontStruct *xfs, const char *name,
					    int mib )
{
    _engines = new QFontEngine*[ engine_array_inc ];
    _engines[0] = new QFontEngineXLFD( xfs, name, mib );
    _count = 1;

    cache_cost = _engines[0]->cache_cost;

    memset( glyphIndices, 0, sizeof( glyphIndices ) );
    memset( glyphAdvances, 0, sizeof( glyphAdvances ) );
    euroIndex = 0;
    euroAdvance = 0;
}

QFontEngineLatinXLFD::~QFontEngineLatinXLFD()
{
    for ( int i = 0; i < _count; ++i ) {
	delete _engines[i];
	_engines[i] = 0;
    }
    delete [] _engines;
    _engines = 0;
}

void QFontEngineLatinXLFD::findEngine( const QChar &ch )
{
    if ( ch.unicode() == 0 ) return;

    static const char *alternate_encodings[] = {
	"iso8859-1",
	"iso8859-2",
	"iso8859-3",
	"iso8859-4",
	"iso8859-9",
	"iso8859-10",
	"iso8859-13",
	"iso8859-14",
	"iso8859-15",
	"hp-roman8"
    };
    static const int mib_count = sizeof( alternate_encodings ) / sizeof( const char * );

    // see if one of the above mibs can map the char we want
    QTextCodec *codec = 0;
    int which = -1;
    int i;
    for ( i = 0; i < mib_count; ++i ) {
	const int mib = qt_mib_for_xlfd_encoding( alternate_encodings[i] );
	bool skip = FALSE;
	for ( int e = 0; e < _count; ++e ) {
	    if ( _engines[e]->cmap() == mib ) {
		skip = TRUE;
		break;
	    }
	}
	if ( skip ) continue;

	codec = QTextCodec::codecForMib( mib );
	if ( codec && codec->canEncode( ch ) ) {
	    which = i;
	    break;
	}
    }

    if ( ! codec || which == -1 )
	return;

    const int enc_id = qt_xlfd_encoding_id( alternate_encodings[which] );
    QFontDef req = fontDef;
    QFontEngine *engine = QFontDatabase::findFont( QFont::Latin, 0, req, enc_id );
    if ( ! engine ) {
	req.family = QString::null;
	engine = QFontDatabase::findFont( QFont::Latin, 0, req, enc_id );
	if ( ! engine ) return;
    }
    engine->setScale( scale() );

    if ( ! ( _count % engine_array_inc ) ) {
	// grow the engines array
	QFontEngine **old = _engines;
	int new_size =
	    ( ( ( _count+engine_array_inc ) / engine_array_inc ) * engine_array_inc );
	_engines = new QFontEngine*[new_size];
	for ( i = 0; i < _count; ++i )
	    _engines[i] = old[i];
	delete [] old;
    }

    _engines[_count] = engine;
    const int hi = _count << 8;
    ++_count;

    unsigned short chars[0x201];
    glyph_t glyphs[0x201];
    advance_t advances[0x201];
    for ( i = 0; i < 0x200; ++i )
	chars[i] = i;
    chars[0x200] = 0x20ac;
    int glyphCount = 0x201;
    engine->stringToCMap( (const QChar *) chars, 0x201, glyphs, advances, &glyphCount, FALSE );

    // merge member data with the above
    for ( i = 0; i < 0x200; ++i ) {
	if ( glyphIndices[i] != 0 || glyphs[i] == 0 ) continue;
	glyphIndices[i] = glyphs[i] >= 0x2100 ? glyphs[i] : hi | glyphs[i];
	glyphAdvances[i] = advances[i];
    }
    if (!euroIndex && glyphs[0x200]) {
	euroIndex = hi | glyphs[0x200];
	euroAdvance = advances[0x200];
    }
}

QFontEngine::Error
QFontEngineLatinXLFD::stringToCMap( const QChar *str, int len, glyph_t *glyphs,
				    advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    int i;
    bool missing = FALSE;
    const QChar *c = str+len;
    glyph_t *g = glyphs+len;
    if ( advances ) {
	int asc = ascent();
	advance_t *a = advances+len;
	if ( mirrored ) {
	    while ( c != str ) {
		--c;
		--g;
		--a;
		if ( c->unicode() < 0x200 ) {
		    unsigned short ch = ::mirroredChar(*c).unicode();
		    *g = glyphIndices[ch];
		    *a = glyphAdvances[ch];
		} else {
		    if ( c->unicode() == 0x20ac ) {
			*g = euroIndex;
			*a = euroAdvance;
		    } else {
			*g = 0;
			*a = asc;
		    }
		}
		missing = ( missing || ( *g == 0 ) );
	    }
	} else {
	    while ( c != str ) {
		--c;
		--g;
		--a;
		if ( c->unicode() < 0x200 ) {
		    *g = glyphIndices[c->unicode()];
		    *a = glyphAdvances[c->unicode()];
		} else {
		    if ( c->unicode() == 0x20ac ) {
			*g = euroIndex;
			*a = euroAdvance;
		    } else {
			*g = 0;
			*a = asc;
		    }
		}
		missing = ( missing || ( *g == 0 ) );
	    }
	}
    } else {
	if ( mirrored ) {
	    while ( c != str ) {
		--c;
		--g;
		*g = ( ( c->unicode() < 0x200 ) ? glyphIndices[::mirroredChar(*c).unicode()]
		       : (c->unicode() == 0x20ac) ? euroIndex : 0 );
		missing = ( missing || ( *g == 0 ) );
	    }
	} else {
	    while ( c != str ) {
		--c;
		--g;
		*g = ( ( c->unicode() < 0x200 ) ? glyphIndices[c->unicode()]
		       : (c->unicode() == 0x20ac) ? euroIndex : 0 );
		missing = ( missing || ( *g == 0 ) );
	    }
	}
    }

    if ( missing ) {
	for ( i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    if ( glyphs[i] != 0 || (uc >= 0x200 && uc != 0x20ac) )
		continue;

	    QFontEngineLatinXLFD *that = (QFontEngineLatinXLFD *) this;
	    that->findEngine( str[i] );
	    glyphs[i] = (uc == 0x20ac ? euroIndex : that->glyphIndices[uc]);
	    if ( advances )
		advances[i] = (uc == 0x20ac ? euroAdvance : glyphAdvances[uc]);
	}
    }

    *nglyphs = len;
    return NoError;
}

void QFontEngineLatinXLFD::draw( QPainter *p, int x, int y, const QTextEngine *engine,
				 const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs ) return;

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    int which = glyphs[0] >> 8;
    if (which > 0x20)
        which = 0;

    int start = 0;
    int end, i;
    for ( end = 0; end < si->num_glyphs; ++end ) {
	int e = glyphs[end] >> 8;
        if (e > 0x20)
            e = 0;
	if ( e == which ) continue;

	// set the high byte to zero
        if (which != 0) {
            for ( i = start; i < end; ++i )
                glyphs[i] = glyphs[i] & 0xff;
        }

	// draw the text
	QScriptItem si2 = *si;
	si2.glyph_data_offset = si->glyph_data_offset + start;
	si2.num_glyphs = end - start;
	_engines[which]->draw( p, x, y, engine, &si2, textFlags );

	// reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 8;
        for ( i = start; i < end; ++i ) {
            glyphs[i] = hi | glyphs[i];
            x += advances[i];
        }

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    if (which != 0) {
        for ( i = start; i < end; ++i )
            glyphs[i] = glyphs[i] & 0xff;
    }
    // draw the text
    QScriptItem si2 = *si;
    si2.glyph_data_offset = si->glyph_data_offset + start;
    si2.num_glyphs = end - start;
    _engines[which]->draw( p, x, y, engine, &si2, textFlags );

    // reset the high byte for all glyphs
    if (which != 0) {
        const int hi = which << 8;
        for ( i = start; i < end; ++i )
            glyphs[i] = hi | glyphs[i];
    }
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox( const glyph_t *glyphs_const,
						   const advance_t *advances,
						   const qoffset_t *offsets,
						   int numGlyphs )
{
    if ( numGlyphs <= 0 ) return glyph_metrics_t();

    glyph_metrics_t overall;

    glyph_t *glyphs = (glyph_t *) glyphs_const;
    int which = glyphs[0] >> 8;
    if (which > 0x20)
        which = 0;

    int start = 0;
    int end, i;
    for ( end = 0; end < numGlyphs; ++end ) {
	int e = glyphs[end] >> 8;
        if (e > 0x20)
            e = 0;
	if ( e == which ) continue;

	// set the high byte to zero
        if (which != 0) {
            for ( i = start; i < end; ++i )
                glyphs[i] = glyphs[i] & 0xff;
        }

	// merge the bounding box for this run
	const glyph_metrics_t gm =
	    _engines[which]->boundingBox( glyphs + start,
					  advances + start,
					  offsets + start,
					  end - start );

	overall.x = QMIN( overall.x, gm.x );
	overall.y = QMIN( overall.y, gm.y );
	overall.width = overall.xoff + gm.width;
	overall.height = QMAX( overall.height + overall.y, gm.height + gm.y ) -
			 QMIN( overall.y, gm.y );
	overall.xoff += gm.xoff;
	overall.yoff += gm.yoff;

	// reset the high byte for all glyphs
        if (which != 0) {
            const int hi = which << 8;
            for ( i = start; i < end; ++i )
                glyphs[i] = hi | glyphs[i];
        }

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    if (which != 0) {
        for ( i = start; i < end; ++i )
            glyphs[i] = glyphs[i] & 0xff;
    }

    // merge the bounding box for this run
    const glyph_metrics_t gm =
	_engines[which]->boundingBox( glyphs + start,
				      advances + start,
				      offsets + start,
				      end - start );

    overall.x = QMIN( overall.x, gm.x );
    overall.y = QMIN( overall.y, gm.y );
    overall.width = overall.xoff + gm.width;
    overall.height = QMAX( overall.height + overall.y, gm.height + gm.y ) -
		     QMIN( overall.y, gm.y );
    overall.xoff += gm.xoff;
    overall.yoff += gm.yoff;

    // reset the high byte for all glyphs
    if (which != 0) {
        const int hi = which << 8;
        for ( i = start; i < end; ++i )
            glyphs[i] = hi | glyphs[i];
    }

    return overall;
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox( glyph_t glyph )
{
    int engine = glyph >> 8;
    if (engine > 0x20)
        engine = 0;
    Q_ASSERT( engine < _count );
    return _engines[engine]->boundingBox( engine > 0 ? glyph & 0xff : glyph );
}

int QFontEngineLatinXLFD::ascent() const
{
    return _engines[0]->ascent();
}

int QFontEngineLatinXLFD::descent() const
{
    return _engines[0]->descent();
}

int QFontEngineLatinXLFD::leading() const
{
    return _engines[0]->leading();
}

int QFontEngineLatinXLFD::maxCharWidth() const
{
    return _engines[0]->maxCharWidth();
}

int QFontEngineLatinXLFD::minLeftBearing() const
{
    return _engines[0]->minLeftBearing();
}

int QFontEngineLatinXLFD::minRightBearing() const
{
    return _engines[0]->minRightBearing();
}

const char *QFontEngineLatinXLFD::name() const
{
    return _engines[0]->name();
}

bool QFontEngineLatinXLFD::canRender( const QChar *string, int len )
{
    bool all = TRUE;
    int i;
    for ( i = 0; i < len; ++i ) {
	if ( string[i].unicode() >= 0x200 ||
	     glyphIndices[string[i].unicode()] == 0 ) {
	    if (string[i].unicode() != 0x20ac || euroIndex == 0)
		all = FALSE;
	    break;
	}
    }

    if ( all )
	return TRUE;

    all = TRUE;
    for ( i = 0; i < len; ++i ) {
	if ( string[i].unicode() >= 0x200 ) {
	    if (string[i].unicode() == 0x20ac) {
		if (euroIndex)
		    continue;

		findEngine(string[i]);
		if (euroIndex)
		    continue;
	    }
	    all = FALSE;
	    break;
	}
	if ( glyphIndices[string[i].unicode()] != 0 ) continue;

	findEngine( string[i] );
	if ( glyphIndices[string[i].unicode()] == 0 ) {
	    all = FALSE;
	    break;
	}
    }

    return all;
}

void QFontEngineLatinXLFD::setScale( double scale )
{
    int i;
    for ( i = 0; i < _count; ++i )
	_engines[i]->setScale( scale );
    unsigned short chars[0x200];
    for ( i = 0; i < 0x200; ++i )
	chars[i] = i;
    int glyphCount = 0x200;
    _engines[0]->stringToCMap( (const QChar *)chars, 0x200,
			       glyphIndices, glyphAdvances, &glyphCount, FALSE );
}


// ------------------------------------------------------------------
// Xft cont engine
// ------------------------------------------------------------------
// #define FONTENGINE_DEBUG

#ifndef QT_NO_XFTFREETYPE
class Q_HackPaintDevice : public QPaintDevice
{
public:
    inline Q_HackPaintDevice() : QPaintDevice( 0 ) {}
    inline XftDraw *xftDrawHandle() const {
	return (XftDraw *)rendhd;
    }

};

#ifdef QT_XFT2
static inline void getGlyphInfo( XGlyphInfo *xgi, XftFont *font, int glyph )
{
    FT_UInt x = glyph;
    XftGlyphExtents( QPaintDevice::x11AppDisplay(), font, &x, 1, xgi );
}
#else
static inline XftFontStruct *getFontStruct( XftFont *font )
{
    if (font->core)
	return 0;
    return font->u.ft.font;
}

static inline void getGlyphInfo(XGlyphInfo *xgi, XftFont *font, int glyph)
{

    XftTextExtents32(QPaintDevice::x11AppDisplay(), font, (XftChar32 *) &glyph, 1, xgi);
}
#endif // QT_XFT2

static inline FT_Face lockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    return XftLockFace( font );
#else
    if (font->core) return 0;
    return font->u.ft.font->face;
#endif // QT_XFT2
}

static inline void unlockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    XftUnlockFace( font );
#else
    Q_UNUSED( font );
#endif // QT_XFT2
}



QFontEngineXft::QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap )
    : _font( font ), _pattern( pattern ), _openType( 0 ), _cmap( cmap )
{
    _face = lockFTFace( _font );

#ifndef QT_XFT2
    XftFontStruct *xftfs = getFontStruct( _font );
    if ( xftfs ) {
	// dirty hack: we set the charmap in the Xftfreetype to -1, so
	// XftFreetype assumes no encoding and really draws glyph
	// indices. The FT_Face still has the Unicode encoding to we
	// can convert from Unicode to glyph index
	xftfs->charmap = -1;
    }
#else
    _cmap = -1;
    // Xft maps Unicode and adobe roman for us.
    for (int i = 0; i < _face->num_charmaps; ++i) {
        FT_CharMap cm = _face->charmaps[i];
//         qDebug("font has charmap %x", cm->encoding);
        if (cm->encoding == ft_encoding_adobe_custom
            || cm->encoding == ft_encoding_symbol) {
//             qDebug("font has adobe custom or ms symbol charmap");
            _cmap = i;
            break;
        }
    }
#endif // QT_XFT2


    cache_cost = _font->height * _font->max_advance_width *
		 ( _face ? _face->num_glyphs : 1024 );

    // if the Xft font is not antialiased, it uses bitmaps instead of
    // 8-bit alpha maps... adjust the cache_cost to reflect this
    Bool antialiased = TRUE;
    if ( XftPatternGetBool( pattern, XFT_ANTIALIAS,
			    0, &antialiased ) == XftResultMatch &&
	 ! antialiased ) {
	cache_cost /= 8;
    }
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    memset( widthCache, 0, sizeof(widthCache) );
    memset( cmapCache, 0, sizeof(cmapCache) );
}

QFontEngineXft::~QFontEngineXft()
{
    delete _openType;
    unlockFTFace( _font );

    XftFontClose( QPaintDevice::x11AppDisplay(),_font );
    XftPatternDestroy( _pattern );
    _font = 0;
    _pattern = 0;
    TransformedFont *trf = transformed_fonts;
    while ( trf ) {
	XftFontClose( QPaintDevice::x11AppDisplay(), trf->xft_font );
	TransformedFont *tmp = trf;
	trf = trf->next;
	delete tmp;
    }
}

#ifdef QT_XFT2
static glyph_t getAdobeCharIndex(XftFont *font, int cmap, uint ucs4)
{
    FT_Face _face = XftLockFace( font );
    FT_Set_Charmap(_face, _face->charmaps[cmap]);
    glyph_t g = FT_Get_Char_Index(_face, ucs4);
    XftUnlockFace(font);
    return g;
}
#endif

QFontEngine::Error QFontEngineXft::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

#ifdef QT_XFT2
    if (_cmap != -1) {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		glyph_t glyph = XftCharIndex(0, _font, uc);
                if (!glyph)
                    glyph = getAdobeCharIndex(_font, _cmap, uc);
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    } else if ( mirrored ) {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = ::mirroredChar(str[i]).unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		if (uc == 0xa0)
		    uc = 0x20;
		glyph_t glyph = XftCharIndex(0, _font, uc);
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    } else {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		if (uc == 0xa0)
		    uc = 0x20;
		glyph_t glyph = XftCharIndex(0, _font, uc);
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    }

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    FT_UInt glyph = *(glyphs + i);
	    advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	    if ( !advances[i] ) {
		XGlyphInfo gi;
		XftGlyphExtents( QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi );
		advances[i] = gi.xOff;
		if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		    ((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
#else
    if ( !_face ) {
	if ( mirrored ) {
	    for ( int i = 0; i < len; i++ )
		glyphs[i] = ::mirroredChar(str[i]).unicode();
	} else {
	    for ( int i = 0; i < len; i++ )
		glyphs[i] = str[i].unicode();
	}
    } else {
	if ( _cmap == 1 ) {
	    // symbol font
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = str[i].unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, uc );
		    if(!glyph && uc < 0x100)
			glyph = FT_Get_Char_Index( _face, uc+0xf000 );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[uc] = glyph;
		}
	    }
	} else if ( mirrored ) {
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = ::mirroredChar(str[i]).unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, uc );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[uc] = glyph;
		}
	    }
	} else {
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = str[i].unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, uc );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[uc] = glyph;
		}
	    }
	}
    }

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    XftChar16 glyph = *(glyphs + i);
	    advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	    if ( !advances[i] ) {
		XGlyphInfo gi;
		XftTextExtents16(QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi);
		advances[i] = gi.xOff;
		if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		    ((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
#endif // QT_XFT2

    *nglyphs = len;
    return NoError;
}


void QFontEngineXft::recalcAdvances( int len, glyph_t *glyphs, advance_t *advances )
{

#ifdef QT_XFT2
    for ( int i = 0; i < len; i++ ) {
	FT_UInt glyph = *(glyphs + i);
	advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !advances[i] ) {
	    XGlyphInfo gi;
	    XftGlyphExtents( QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi );
	    advances[i] = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
#else
    for ( int i = 0; i < len; i++ ) {
	XftChar16 glyph = *(glyphs + i);
	advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !advances[i] ) {
	    XGlyphInfo gi;
	    XftTextExtents16(QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi);
	    advances[i] = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
    }
    if ( _scale != 1. ) {
	for ( int i = 0; i < len; i++ )
	    advances[i] = qRound(advances[i]*_scale);
    }
#endif // QT_XFT2
}

//#define FONTENGINE_DEBUG
void QFontEngineXft::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs )
	return;

    Display *dpy = QPaintDevice::x11AppDisplay();

    int xorig = x;
    int yorig = y;

    XftFont *fnt = _font;
    bool transform = FALSE;
    if ( p->txop >= QPainter::TxScale || p->rop != Qt::CopyROP) {
        bool can_scale = (_face->face_flags & FT_FACE_FLAG_SCALABLE) && p->rop == Qt::CopyROP;
        double size = (p->m11()*p->m22() - p->m12()*p->m21())*_scale*_scale*fontDef.pixelSize*fontDef.pixelSize;
        if (size > 256*256)
            can_scale = FALSE;
	if (!can_scale) {
	    // font doesn't support transformations, need to do it by hand
            drawScaled(x, y, engine, si, textFlags, dpy, p->gc, p->device(), this, p->xmat);
	    return;
	}

	XftMatrix *mat = 0;
	XftPatternGetMatrix( _pattern, XFT_MATRIX, 0, &mat );
	XftMatrix m2;
	m2.xx = p->m11()*_scale;
	m2.xy = -p->m21()*_scale;
	m2.yx = -p->m12()*_scale;
	m2.yy = p->m22()*_scale;

	// check if we have it cached
	TransformedFont *trf = transformed_fonts;
	TransformedFont *prev = 0;
	int i = 0;
	while ( trf ) {
	    if ( trf->xx == (float)m2.xx &&
		 trf->xy == (float)m2.xy &&
		 trf->yx == (float)m2.yx &&
		 trf->yy == (float)m2.yy )
		break;
	    TransformedFont *tmp = trf;
	    trf = trf->next;
	    if (i > 10) {
		XftFontClose( QPaintDevice::x11AppDisplay(), tmp->xft_font );
		delete tmp;
		prev->next = trf;
	    } else {
		prev = tmp;
	    }
	    ++i;
	}
	if ( trf ) {
	    if ( prev ) {
		// move to beginning of list
		prev->next = trf->next;
		trf->next = transformed_fonts;
		transformed_fonts = trf;
	    }
	    fnt = trf->xft_font;
	} else {
	    if ( mat )
		XftMatrixMultiply( &m2, &m2, mat );

            XftPattern *pattern = XftPatternDuplicate( _pattern );
	    XftPatternDel( pattern, XFT_MATRIX );
	    XftPatternAddMatrix( pattern, XFT_MATRIX, &m2 );

	    fnt = XftFontOpenPattern( dpy, pattern );
#ifndef QT_XFT2
	    XftFontStruct *xftfs = getFontStruct( fnt );
	    if ( xftfs ) {
		// dirty hack: we set the charmap in the Xftfreetype to -1, so
		// XftFreetype assumes no encoding and really draws glyph
		// indices. The FT_Face still has the Unicode encoding to we
		// can convert from Unicode to glyph index
		xftfs->charmap = -1;
	    }
#endif // QT_XFT2
	    TransformedFont *trf = new TransformedFont;
	    trf->xx = (float)m2.xx;
	    trf->xy = (float)m2.xy;
	    trf->yx = (float)m2.yx;
	    trf->yy = (float)m2.yy;
	    trf->xft_font = fnt;
	    trf->next = transformed_fonts;
	    transformed_fonts = trf;
	}
	transform = TRUE;
    } else if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    const QColor &pen = p->cpen.color();
    XftDraw *draw = ((Q_HackPaintDevice *)p->pdev)->xftDrawHandle();

    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
#ifdef FONTENGINE_DEBUG
    qDebug("===== drawing %d glyphs reverse=%s ======", si->num_glyphs, si->analysis.bidiLevel % 2?"true":"false" );
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, si->num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int yp = y;
    int xp = x;
#endif

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, si->width, textFlags );


    if ( si->isSpace )
	return;

    if ( transform || si->hasPositioning ) {
	if ( si->analysis.bidiLevel % 2 ) {
	    int i = si->num_glyphs;
	    while( i-- ) {
		int xp = x + offsets[i].x;
		int yp = y + offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
#ifdef QT_XFT2
		FT_UInt glyph = *(glyphs + i);
		if (xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawGlyphs( draw, &col, fnt, xp, yp, &glyph, 1 );
#else
		if (xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawString16( draw, &col, fnt, xp, yp, (XftChar16 *) (glyphs+i), 1);
#endif // QT_XFT2
#ifdef FONTENGINE_DEBUG
		glyph_metrics_t gi = boundingBox( glyphs[i] );
		p->drawRect( x+offsets[i].x+gi.x, y+offsets[i].y+100+gi.y, gi.width, gi.height );
		p->drawLine( x+offsets[i].x, y + 150 + 5*i , x+offsets[i].x+advances[i], y + 150 + 5*i );
		p->drawLine( x+offsets[i].x, y + 152 + 5*i , x+offsets[i].x+gi.xoff, y + 152 + 5*i );
		qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=%d", i, gi.x, gi.y, gi.width, gi.height,
		       gi.xoff, gi.yoff, offsets[i].x, offsets[i].y, advances[i]);
#endif
		x += advances[i];
	    }
	} else {
	    int i = 0;
	    while ( i < si->num_glyphs ) {
		int xp = x + offsets[i].x;
		int yp = y + offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
#ifdef QT_XFT2
		FT_UInt glyph = *(glyphs + i);
		if (xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawGlyphs( draw, &col, fnt, xp, yp, &glyph, 1 );
#else
		if (xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawString16( draw, &col, fnt, xp, yp, (XftChar16 *) (glyphs+i), 1 );
#endif // QT_XFT2
		// 	    qDebug("advance = %d/%d", adv.x, adv.y );
		x += advances[i];
		i++;
	    }
	}
    } else {
	// Xft has real trouble drawing the glyphs on their own.
	// Drawing them as one string increases performance significantly.
#ifdef QT_XFT2
	// #### we should use a different method anyways on Xft2
	FT_UInt g[64];
	int gl = 0;
	while (gl < si->num_glyphs) {
	    int toDraw = QMIN(64, si->num_glyphs-gl);
	    int adv = 0;
	    if ( si->analysis.bidiLevel % 2 ) {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[si->num_glyphs-1-(gl+i)];
		    adv += advances[si->num_glyphs-1-(gl+i)];
		}
	    } else {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[gl+i];
		    adv += advances[gl+i];
		}
	    }
	    if (x + adv < SHRT_MAX && x > SHRT_MIN)
		XftDrawGlyphs( draw, &col, fnt, x, y, g, toDraw );
	    gl += toDraw;
	    x += adv;
	}
#else
	XftChar16 g[64];
	int gl = 0;
	while (gl < si->num_glyphs) {
	    int toDraw = QMIN(64, si->num_glyphs-gl);
	    int adv = 0;
	    if ( si->analysis.bidiLevel % 2 ) {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[si->num_glyphs-1-(gl+i)];
		    adv += advances[si->num_glyphs-1-(gl+i)];
		}
	    } else {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[gl+i];
		    adv += advances[gl+i];
		}
	    }
	    if (x + adv < SHRT_MAX && x > SHRT_MIN)
		XftDrawString16( draw, &col, fnt, x, y, g, toDraw );
	    gl += toDraw;
	    x += adv;
	}
#endif // QT_XFT2
    }

#ifdef FONTENGINE_DEBUG
    if ( !si->analysis.bidiLevel % 2 ) {
	x = xp;
	y = yp;
	p->save();
	p->setPen( Qt::red );
	for ( int i = 0; i < si->num_glyphs; i++ ) {
	    glyph_metrics_t ci = boundingBox( glyphs[i] );
	    p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	    qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=%d", i, ci.x, ci.y, ci.width, ci.height,
		   ci.xoff, ci.yoff, offsets[i].x, offsets[i].y, advances[i]);
	    x += advances[i];
	}
	p->restore();
    }
#endif
}

glyph_metrics_t QFontEngineXft::boundingBox( const glyph_t *glyphs, const advance_t *advances, const qoffset_t *offsets, int numGlyphs )
{
    XGlyphInfo xgi;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    if (_scale != 1) {
        for (int i = 0; i < numGlyphs; i++) {
            getGlyphInfo( &xgi, _font, glyphs[i] );
            int x = overall.xoff + offsets[i].x - xgi.x;
            int y = overall.yoff + offsets[i].y - xgi.y;
            overall.x = QMIN( overall.x, x );
            overall.y = QMIN( overall.y, y );
            xmax = QMAX( xmax, x + xgi.width );
            ymax = QMAX( ymax, y + xgi.height );
            overall.xoff += qRound(advances[i]/_scale);
        }
	overall.x = qRound(overall.x * _scale);
	overall.y = qRound(overall.y * _scale);
	overall.xoff = qRound(overall.xoff * _scale);
	overall.yoff = qRound(overall.yoff * _scale);
	xmax = qRound(xmax * _scale);
	ymax = qRound(ymax * _scale);
    } else {
        for (int i = 0; i < numGlyphs; i++) {
            getGlyphInfo( &xgi, _font, glyphs[i] );
            int x = overall.xoff + offsets[i].x - xgi.x;
            int y = overall.yoff + offsets[i].y - xgi.y;
            overall.x = QMIN( overall.x, x );
            overall.y = QMIN( overall.y, y );
            xmax = QMAX( xmax, x + xgi.width );
            ymax = QMAX( ymax, y + xgi.height );
            overall.xoff += advances[i];
        }
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;
    return overall;
}

glyph_metrics_t QFontEngineXft::boundingBox( glyph_t glyph )
{
    XGlyphInfo xgi;
    getGlyphInfo( &xgi, _font, glyph );
    glyph_metrics_t gm = glyph_metrics_t( -xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff );
    if ( _scale != 1. ) {
	gm.x = qRound(gm.x * _scale);
	gm.y = qRound(gm.y * _scale);
	gm.height = qRound(gm.height * _scale);
	gm.width = qRound(gm.width * _scale);
	gm.xoff = qRound(gm.xoff * _scale);
	gm.yoff = qRound(gm.yoff * _scale);
    }
    return gm;
}



int QFontEngineXft::ascent() const
{
    return qRound(_font->ascent*_scale);
}

int QFontEngineXft::descent() const
{
    return qRound((_font->descent-1)*_scale);
}

// #### use Freetype to determine this
int QFontEngineXft::leading() const
{
    int l = qRound(QMIN( _font->height - (_font->ascent + _font->descent),
			((_font->ascent + _font->descent) >> 4)*_scale ));
    return (l > 0) ? l : 1;
}

// #### use Freetype to determine this
int QFontEngineXft::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 ) lw = 2;
    if ( lw == 0 ) lw = 1;

    return lw;
}

// #### use Freetype to determine this
int QFontEngineXft::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}

int QFontEngineXft::maxCharWidth() const
{
    return qRound(_font->max_advance_width*_scale);
}

static const ushort char_table[] = {
	40,
	67,
	70,
	75,
	86,
	88,
	89,
	91,
	102,
	114,
	124,
	127,
	205,
	645,
	884,
	922,
	1070,
	12386
};

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


int QFontEngineXft::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN )
	minRightBearing(); // calculates both

    return lbearing;
}

int QFontEngineXft::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
	QFontEngineXft *that = (QFontEngineXft *)this;
	that->lbearing = that->rbearing = 0;
	QChar *ch = (QChar *)char_table;
	glyph_t glyphs[char_table_entries];
	int ng = char_table_entries;
	stringToCMap(ch, char_table_entries, glyphs, 0, &ng, FALSE);
	while (--ng) {
	    if (glyphs[ng]) {
		glyph_metrics_t gi = that->boundingBox( glyphs[ng] );
		if (gi.xoff) {
		    that->lbearing = QMIN(lbearing, gi.x);
		    that->rbearing = QMIN(rbearing, gi.xoff - gi.x - gi.width);
		}
	    }
	}
    }

    return rbearing;
}

int QFontEngineXft::cmap() const
{
    return _cmap;
}

const char *QFontEngineXft::name() const
{
    return "xft";
}

void QFontEngineXft::setScale( double scale )
{
    _scale = scale;
}

bool QFontEngineXft::canRender( const QChar *string, int len )
{
    bool allExist = TRUE;

#ifdef QT_XFT2
    if (_cmap != -1) {
        for ( int i = 0; i < len; i++ ) {
            if (!XftCharExists(0, _font, string[i].unicode())
                && getAdobeCharIndex(_font, _cmap, string[i].unicode()) == 0) {
                allExist = FALSE;
                break;
            }
        }
    } else {
        for ( int i = 0; i < len; i++ ) {
            if (!XftCharExists(0, _font, string[i].unicode())) {
                allExist = FALSE;
                break;
            }
        }
    }
#else
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs, FALSE ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs, FALSE );
    }

    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !XftGlyphExists(QPaintDevice::x11AppDisplay(), _font, g[i]) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( g != glyphs )
	free( g );
#endif // QT_XFT2

    return allExist;
}

QOpenType *QFontEngineXft::openType() const
{
//     qDebug("openTypeIface requested!");
    if ( _openType )
	return _openType;

    if ( !_face || ! FT_IS_SFNT( _face ) )
	return 0;

    QFontEngineXft *that = (QFontEngineXft *)this;
    that->_openType = new QOpenType(that);
    return _openType;
}


QFontEngine::Type QFontEngineXft::type() const
{
    return Xft;
}
#endif


//  --------------------------------------------------------------------------------------------------------------------
// Open type support
//  --------------------------------------------------------------------------------------------------------------------

#ifndef QT_NO_XFTFREETYPE

// #define OT_DEBUG

#ifdef OT_DEBUG
static inline char *tag_to_string( FT_ULong tag )
{
    static char string[5];
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
    return string;
}
#endif

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG( 'D', 'F', 'L', 'T' )

static const unsigned int supported_scripts [] = {
// 	// European Alphabetic Scripts
// 	Latin,
    FT_MAKE_TAG( 'l', 'a', 't', 'n' ),
// 	Greek,
    FT_MAKE_TAG( 'g', 'r', 'e', 'k' ),
// 	Cyrillic,
    FT_MAKE_TAG( 'c', 'y', 'r', 'l' ),
// 	Armenian,
        FT_MAKE_TAG( 'a', 'r', 'm', 'n' ),
// 	Georgian,
    FT_MAKE_TAG( 'g', 'e', 'o', 'r' ),
// 	Runic,
    FT_MAKE_TAG( 'r', 'u', 'n', 'r' ),
// 	Ogham,
    FT_MAKE_TAG( 'o', 'g', 'a', 'm' ),
// 	SpacingModifiers,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	CombiningMarks,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),

// 	// Middle Eastern Scripts
// 	Hebrew,
    FT_MAKE_TAG( 'h', 'e', 'b', 'r' ),
// 	Arabic,
    FT_MAKE_TAG( 'a', 'r', 'a', 'b' ),
// 	Syriac,
    FT_MAKE_TAG( 's', 'y', 'r', 'c' ),
// 	Thaana,
    FT_MAKE_TAG( 't', 'h', 'a', 'a' ),

// 	// South and Southeast Asian Scripts
// 	Devanagari,
    FT_MAKE_TAG( 'd', 'e', 'v', 'a' ),
// 	Bengali,
    FT_MAKE_TAG( 'b', 'e', 'n', 'g' ),
// 	Gurmukhi,
    FT_MAKE_TAG( 'g', 'u', 'r', 'u' ),
// 	Gujarati,
    FT_MAKE_TAG( 'g', 'u', 'j', 'r' ),
// 	Oriya,
    FT_MAKE_TAG( 'o', 'r', 'y', 'a' ),
// 	Tamil,
    FT_MAKE_TAG( 't', 'a', 'm', 'l' ),
// 	Telugu,
    FT_MAKE_TAG( 't', 'e', 'l', 'u' ),
// 	Kannada,
    FT_MAKE_TAG( 'k', 'n', 'd', 'a' ),
// 	Malayalam,
    FT_MAKE_TAG( 'm', 'l', 'y', 'm' ),
// 	Sinhala,
    // ### could not find any OT specs on this
    FT_MAKE_TAG( 's', 'i', 'n', 'h' ),
// 	Thai,
    FT_MAKE_TAG( 't', 'h', 'a', 'i' ),
// 	Lao,
    FT_MAKE_TAG( 'l', 'a', 'o', ' ' ),
// 	Tibetan,
    FT_MAKE_TAG( 't', 'i', 'b', 't' ),
// 	Myanmar,
    FT_MAKE_TAG( 'm', 'y', 'm', 'r' ),
// 	Khmer,
    FT_MAKE_TAG( 'k', 'h', 'm', 'r' ),

// 	// East Asian Scripts
// 	Han,
    FT_MAKE_TAG( 'h', 'a', 'n', 'i' ),
// 	Hiragana,
    FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Katakana,
    FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Hangul,
    FT_MAKE_TAG( 'h', 'a', 'n', 'g' ),
// 	Bopomofo,
    FT_MAKE_TAG( 'b', 'o', 'p', 'o' ),
// 	Yi,
    FT_MAKE_TAG( 'y', 'i', ' ', ' ' ),

// 	// Additional Scripts
// 	Ethiopic,
    FT_MAKE_TAG( 'e', 't', 'h', 'i' ),
// 	Cherokee,
    FT_MAKE_TAG( 'c', 'h', 'e', 'r' ),
// 	CanadianAboriginal,
    FT_MAKE_TAG( 'c', 'a', 'n', 's' ),
// 	Mongolian,
    FT_MAKE_TAG( 'm', 'o', 'n', 'g' ),

// 	// Symbols
// 	CurrencySymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	LetterlikeSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	NumberForms,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	MathematicalOperators,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	TechnicalSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	GeometricSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	MiscellaneousSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	EnclosedAndSquare,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	Braille,
    FT_MAKE_TAG( 'b', 'r', 'a', 'i' ),

//                Unicode, should be used
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' )
    // ### where are these?
// 	FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
//     FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
    // ### Hangul Jamo
//     FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
};

QOpenType::QOpenType(QFontEngineXft *fe)
    : xft(fe), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    face = xft->face();
    hasGDef = hasGSub = hasGPos = TRUE;
    str = tmp = 0;
    positions = 0;
    tmpAttributes = 0;
    tmpLogClusters = 0;
}

QOpenType::~QOpenType()
{
    if ( gpos )
	TT_Done_GPOS_Table( gpos );
    if ( gsub )
	TT_Done_GSUB_Table( gsub );
    if ( gdef )
	TT_Done_GDEF_Table( gdef );
    if ( str )
	TT_GSUB_String_Done( str );
    if ( tmp )
	TT_GSUB_String_Done( tmp );
    if (positions)
	free(positions);
    if ( tmpAttributes )
	free(tmpAttributes);
    if (tmpLogClusters)
	free(tmpLogClusters);
}

bool QOpenType::supportsScript( unsigned int script )
{
    if ( current_script == supported_scripts[script] )
	return TRUE;

#ifdef OT_DEBUG
    qDebug("trying to load tables for script %d (%s))", script, tag_to_string( supported_scripts[script] ));
#endif

    FT_Error error;
    if ( !gdef ) {
	if ( (error = TT_Load_GDEF_Table( face, &gdef )) ) {
//  	    qDebug("error loading gdef table: %d", error );
	    hasGDef = FALSE;
	}
    }

    if ( !gsub ) {
	if ( (error = TT_Load_GSUB_Table( face, &gsub, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
//  		qDebug("error loading gsub table: %d", error );
		return FALSE;
	    } else {
//  		qDebug("face doesn't have a gsub table" );
		hasGSub = FALSE;
	    }
	}
    }

    if ( !gpos ) {
	if ( (error = TT_Load_GPOS_Table( face, &gpos, gdef )) ) {
//  		qDebug("error loading gpos table: %d", error );
	    hasGPos = FALSE;
	}
    }

    if ( loadTables( script ) ) {
	return TRUE;
    }
    return FALSE;
}

bool QOpenType::loadTables( FT_ULong script)
{
    assert( script < QFont::Unicode );
    // find script in our list of supported scripts.
    unsigned int stag = supported_scripts[script];

    FT_Error error = TT_GSUB_Select_Script( gsub, stag, &script_index );
    if ( error ) {
#ifdef OT_DEBUG
 	qDebug("could not select script %d: %d", (int)script, error );
#endif
	if ( stag == DefaultScript ) {
	    // try to load default language system
	    error = TT_GSUB_Select_Script( gsub, DefaultScript, &script_index );
	    if ( error )
		return FALSE;
	} else {
	    return FALSE;
	}
    }
    script = stag;

#ifdef OT_DEBUG
    qDebug("script %s has script index %d", tag_to_string(script), script_index );
#endif

#ifdef OT_DEBUG
    TTO_FeatureList featurelist = gsub->FeatureList;
    int numfeatures = featurelist.FeatureCount;
    qDebug("gsub table has %d features", numfeatures );
    for( int i = 0; i < numfeatures; i++ ) {
	TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
    }
#endif
    if ( hasGPos ) {
	FT_UShort script_index;
	error = TT_GPOS_Select_Script( gpos, script, &script_index );
	if ( error ) {
// 	    qDebug("could not select script in gpos table: %d", error );
	    return TRUE;
	}

	TTO_FeatureList featurelist = gpos->FeatureList;

	int numfeatures = featurelist.FeatureCount;

#ifdef OT_DEBUG
 	qDebug("gpos table has %d features", numfeatures );
#endif

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );

#ifdef OT_DEBUG
 	    qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
#endif
	}


    }

    current_script = script;

    return TRUE;
}

void QOpenType::init(glyph_t *glyphs, GlyphAttributes *glyphAttributes, int num_glyphs,
		     unsigned short *logClusters, int len, int /*char_offset*/)
{
    if ( !str )
	TT_GSUB_String_New(&str);
    if ( str->allocated < (uint)num_glyphs )
	TT_GSUB_String_Allocate( str, num_glyphs );
    if ( !tmp )
	TT_GSUB_String_New(&tmp);
    if ( tmp->allocated < (uint)num_glyphs )
	TT_GSUB_String_Allocate( tmp, num_glyphs );
    tmp->length = 0;

    length = len;

    Q_ASSERT(len == num_glyphs);
    memcpy(str->string, glyphs, num_glyphs*sizeof(unsigned short));

    for (int i = 0; i < num_glyphs; ++i)
	str->character_index[i] = i;

#ifdef OT_DEBUG
    qDebug("-----------------------------------------");
    qDebug("log clusters before shaping:");
    for (int j = 0; j < length; j++)
	qDebug("    log[%d] = %d", j, logClusters[j] );
    qDebug("original glyphs:");
    for (int i = 0; i < num_glyphs; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmb: %d", str->string[i], str->character_index[i], glyphAttributes[i].mark, glyphAttributes[i].combiningClass);
#endif
    str->length = num_glyphs;
    orig_nglyphs = num_glyphs;

    tmpLogClusters = (unsigned short *) realloc( tmpLogClusters, length*sizeof(unsigned short) );
    memcpy( tmpLogClusters, logClusters, length*sizeof(unsigned short) );
    tmpAttributes = (GlyphAttributes *) realloc( tmpAttributes, num_glyphs*sizeof(GlyphAttributes) );
    memcpy( tmpAttributes, glyphAttributes, num_glyphs*sizeof(GlyphAttributes) );
}

void QOpenType::applyGSUBFeature(unsigned int featureTag, bool *where)
{
    FT_UShort feature_index;
    FT_Error err = TT_GSUB_Select_Feature( gsub, featureTag, script_index, 0xffff, &feature_index);
    if (err) {
#ifdef OT_DEBUG
// 	qDebug("feature %s not covered by table or language system", tag_to_string( featureTag ));
#endif
	return;
    }

#ifdef OT_DEBUG
    qDebug("applying GSUB feature %s with index %d", tag_to_string( featureTag ), feature_index);
#endif

    unsigned char w[256];
    unsigned char *where_to_apply = w;
    if (str->length > 255)
	where_to_apply = (unsigned char *)malloc(str->length*sizeof(unsigned char));

    memset(where_to_apply, 1, str->length);
    if (where) {
	for (uint i = 0; i < str->length; ++i) {
	    where_to_apply[i] = where[str->character_index[i]];
	}
#ifdef OT_DEBUG
// 	for (int i = 0; i < (int)str->length; ++i)
// 	    qDebug("   apply=%s", where_to_apply[i] ? "true" : "false");
#endif
    }

    TT_GSUB_Apply_Feature(gsub, feature_index, where_to_apply, &str, &tmp);

    if (w != where_to_apply)
	free(where_to_apply);

#ifdef OT_DEBUG
    qDebug("after applying:");
    for ( int i = 0; i < (int)str->length; i++) {
      qDebug("   %4x", str->string[i]);
    }
#endif
    positioned = FALSE;
}


extern void q_heuristicPosition( QTextEngine *engine, QScriptItem *item );

void QOpenType::applyGPOSFeatures()
{
#ifdef OT_DEBUG
    qDebug("applying GPOS features");
#endif
    // currently just apply all features

    if ( hasGPos ) {
	positions = (TTO_GPOS_Data *) realloc( positions, str->length*sizeof(TTO_GPOS_Data) );
	memset(positions, 0, str->length*sizeof(TTO_GPOS_Data));

	TTO_FeatureList featurelist = gpos->FeatureList;
	int numfeatures = featurelist.FeatureCount;

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    FT_Error error = TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );
	    if (error != FT_Err_Ok)
		continue;

#ifdef OT_DEBUG
	    qDebug("applying POS feature %s with index %d", tag_to_string( r->FeatureTag ), feature_index);
#endif

	    str->pos = 0;
	    // ensure the face has the correct size and matrix set
	    face = lockFTFace(xft->_font);
	    TT_GPOS_Apply_Feature( face, gpos, feature_index, FT_LOAD_DEFAULT, str, &positions, FALSE, FALSE );
	    unlockFTFace(xft->_font);
	}
    }
    positioned = TRUE;
}

const int *QOpenType::mapping(int &len)
{
    len = str->length;
    return str->character_index;
}

void QOpenType::appendTo(QTextEngine *engine, QScriptItem *si, bool doLogClusters)
{
#ifdef OT_DEBUG
    qDebug("QOpenType::finalize:");
#endif
    // make sure we have enough space to write everything back
    engine->ensureSpace( si->num_glyphs + str->length );

    glyph_t *glyphs = engine->glyphs( si ) + si->num_glyphs;
    advance_t *advances = engine->advances(si) + si->num_glyphs;
    qoffset_t *offsets = engine->offsets(si) + si->num_glyphs;
    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si ) + si->num_glyphs;

    memcpy(glyphs, str->string, str->length*sizeof(glyph_t));
    if (doLogClusters) {
	// we can't do this for indic, as we pass the stuf in syllables and it's easier to do it in the shaper.
	unsigned short *logClusters = engine->logClusters( si );
	int clusterStart = 0;
	int oldCi = 0;
	for ( int i = 0; i < (int)str->length; i++ ) {
	    int ci = str->character_index[i];
	    glyphAttributes[i] = tmpAttributes[ci];
	    // 	qDebug("   ci[%d] = %d mark=%d, cmb=%d, cs=%d tmplc=%d",
	    // 	       i, ci, glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart,  tmpLogClusters[ci]);
	    if ( !glyphAttributes[i].mark && glyphAttributes[i].clusterStart && ci != oldCi ) {
		for ( int j = oldCi; j < ci; j++ )
		    logClusters[j] = clusterStart;
		clusterStart = i;
		oldCi = ci;
	    }
	}
	for ( int j = oldCi; j < length; j++ )
	    logClusters[j] = clusterStart;
    }

    // calulate the advances for the shaped glyphs
//     qDebug("unpositioned: ");
    ((QFontEngineXft *)si->fontEngine)->recalcAdvances( str->length, glyphs, advances );
    for ( int i = 0; i < (int)str->length; i++ ) {
	if ( glyphAttributes[i].mark )
	    advances[i] = 0;
// 	    qDebug("   adv=%d (mark=%d)", advances[i], glyphAttributes[i].mark);
    }
    si->num_glyphs += str->length;

    // positioning code:
    if ( hasGPos && positioned) {
	float scale = si->fontEngine->scale();
//   	qDebug("positioned glyphs:" );
	for ( int i = 0; i < (int)str->length; i++) {
// 	    qDebug("    %d:\t orig advance: %d\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
// 		   advances[i], (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6 ),
// 		   (int)(positions[i].x_pos >> 6 ), (int)(positions[i].y_pos >> 6),
// 		   positions[i].back, positions[i].new_advance );
	    // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
	    if ( positions[i].new_advance ) {
		advances[i] = si->analysis.bidiLevel % 2 ?
			      -qRound((positions[i].x_advance >> 6)*scale) :
			      qRound((positions[i].x_advance >> 6)*scale);
		//advances[i].y = -positions[i].y_advance >> 6;
	    } else {
		advances[i] += si->analysis.bidiLevel % 2 ?
			      -qRound((positions[i].x_advance >> 6)*scale) :
			      qRound((positions[i].x_advance >> 6)*scale);
		//advances[i].y -= positions[i].y_advance >> 6;
	    }
	    offsets[i].x = qRound((positions[i].x_pos >> 6)*scale);
	    offsets[i].y = -qRound((positions[i].y_pos >> 6)*scale);
	    int back = positions[i].back;
	    if ( si->analysis.bidiLevel % 2 ) {
		while ( back-- )
		    offsets[i].x -= advances[i-back];
	    } else {
		while ( back )
		    offsets[i].x -= advances[i-(back--)];
	    }
// 	    qDebug("   ->\tadv=%d\tpos=(%d/%d)",
// 		   advances[i], offsets[i].x, offsets[i].y );
	}
	si->hasPositioning = TRUE;
    } else {
	q_heuristicPosition( engine, si );
    }

#ifdef OT_DEBUG
    qDebug("log clusters after shaping:");
    if (doLogClusters) {
	for (int j = 0; j < length; j++)
	    qDebug("    log[%d] = %d", j, engine->logClusters(si)[j] );
    }
    qDebug("final glyphs:");
    for (int i = 0; i < (int)str->length; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmp: %d, clusterStart: %d width=%d",
	       glyphs[i], str->character_index[i], glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart,
	       advances[i]);
    qDebug("-----------------------------------------");
#endif
}

#endif
