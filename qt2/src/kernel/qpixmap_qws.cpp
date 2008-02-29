/****************************************************************************
** $Id: qt/src/kernel/qpixmap_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QPixmap class for FB
**
** Created : 991026
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qglobal.h"

#include "qbitmap.h"
#include "qpaintdevicemetrics.h"
#include "qimage.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qptrdict.h"
#include "qwsdisplay_qws.h"
#include "qgfx_qws.h"
#include <stdlib.h>

#include "qmemorymanager_qws.h"

extern int *qt_next_pixmap_id;

/*****************************************************************************
  Internal functions
 *****************************************************************************/

extern const uchar *qt_get_bitflip_array();		// defined in qimage.cpp

static uchar *flip_bits( const uchar *bits, int len )
{
    register const uchar *p = bits;
    const uchar *end = p + len;
    uchar *newdata = new uchar[len];
    uchar *b = newdata;
    const uchar *f = qt_get_bitflip_array();
    while ( p < end )
	*b++ = f[*p++];
    return newdata;
}

// Returns position of highest bit set or -1 if none
/*
static int highest_bit( uint v )
{
    int i;
    uint b = (uint)1 << 31;
    for ( i=31; ((b & v) == 0) && i>=0;	 i-- )
	b >>= 1;
    return i;
}
*/

// Returns position of lowest set bit in 'v' as an integer (0-31), or -1
/*
static int lowest_bit( uint v )
{
    int i;
    ulong lb;
    lb = 1;
    for (i=0; ((v & lb) == 0) && i<32;  i++, lb<<=1);
    return i==32 ? -1 : i;
}
*/

// Counts the number of bits set in 'v'
/*
static uint n_bits( uint v )
{
    int i = 0;
    while ( v ) {
	v = v & (v - 1);
	i++;
    }
    return i;
}
*/

/*
static uint *red_scale_table   = 0;
static uint *green_scale_table = 0;
static uint *blue_scale_table  = 0;

static void cleanup_scale_tables()
{
    delete[] red_scale_table;
    delete[] green_scale_table;
    delete[] blue_scale_table;
}
*/

/*
  Could do smart bitshifting, but the "obvious" algorithm only works for
  nBits >= 4. This is more robust.
*/
/*
static void build_scale_table( uint **table, uint nBits )
{
    if ( nBits > 7 ) {
#if defined(CHECK_RANGE)
	qWarning( "build_scale_table: internal error, nBits = %i", nBits );
#endif
	return;
    }
    if (!*table) {
	static bool firstTable = TRUE;
	if ( firstTable ) {
	    qAddPostRoutine( cleanup_scale_tables );
	    firstTable = FALSE;
	}
	*table = new uint[256];
    }
    int   maxVal   = (1 << nBits) - 1;
    int   valShift = 8 - nBits;
    int i;
    for( i = 0 ; i < maxVal + 1 ; i++ )
	(*table)[i << valShift] = i*255/maxVal;
}
*/

static QList<QShared> *qws_pixmapData = 0;
static bool qws_trackPixmapData = TRUE;

class QwsPixmap : public QPixmap
{
public:
    QwsPixmap() : QPixmap() {}
    static void mapPixmaps( bool from );
    static QPtrDict<QImage> *images;
};

QPtrDict<QImage> *QwsPixmap::images = 0;

void QwsPixmap::mapPixmaps( bool from )
{
    if ( !qws_pixmapData )
	return;
    if ( !images )
	images = new QPtrDict<QImage>;
    qws_trackPixmapData = FALSE;
    QListIterator<QShared> it( *qws_pixmapData );
    while ( it.current() ) {
	QPixmapData *d = (QPixmapData*)it.current();
	++it;
	if ( d->w && d->h ) {
	    if ( from ) {
		QwsPixmap p;
		QPixmapData *tmp = p.data;
		p.data = d;
		QImage *img = new QImage(p.convertToImage());
		images->insert( d, img );
		p.data = tmp;
	    } else {
		QImage *img = images->take( d );
		if ( img ) {
		    if ( d->clut )
			delete [] d->clut;
		    if ( memorymanager )
			memorymanager->deletePixmap(d->id);
		    QwsPixmap p;
		    p.convertFromImage( *img );
		    int cnt = d->count-1;
		    p.data->mask = d->mask;
		    *d = *p.data;
		    while (cnt > 0) {
			d->ref();
			--cnt;
		    }
		    delete img;
		    delete p.data;
		    p.data = 0;
		}
	    }
	}
    }
    if ( !from )
	images->clear();
    qws_trackPixmapData = TRUE;
}

void qws_mapPixmaps( bool from )
{
    QwsPixmap::mapPixmaps( from );
}

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/

void QPixmap::init( int w, int h, int d, bool bitmap, Optimization optim )
{
    int dd = defaultDepth();

    if ( !qws_pixmapData )
	qws_pixmapData = new QList<QShared>;

    if ( optim == DefaultOptim )		// use default optimization
	optim = defOptim;

    data = new QPixmapData;
    CHECK_PTR( data );

    if ( qws_trackPixmapData )
	qws_pixmapData->append( data );

    memset( data, 0, sizeof(QPixmapData) );
    data->id=0;
    data->count  = 1;
    data->uninit = TRUE;
    data->bitmap = bitmap;
    data->ser_no = ++(*qt_next_pixmap_id);
    data->optim	 = optim;
    data->clut=0;
    data->numcols = 0;
    data->hasAlpha = FALSE;

    if ( d > 0 && !qwsDisplay()->supportsDepth(d) )
	d = dd; // caller asked for an unsupported depth

    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( d == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( d < 0 || d == dd )		// def depth pixmap
	data->d = dd;
    else
	data->d = d;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	data->id = 0;
	data->w = 0;
	data->h = 0;
#if defined(CHECK_RANGE)
	if ( !make_null ) {
	    qWarning( "QPixmap: Invalid pixmap parameters, %d %d %d",w,h,data->d);
	    abort();
	}
#endif
	return;
    }
    data->w = w;
    data->h = h;

    if(data->d<=8) {
	if ( qt_screen->numCols() ) {
	    data->numcols = qt_screen->numCols();
	    data->clut = new QRgb[qt_screen->numCols()];
	    for ( int i = 0; i < qt_screen->numCols(); i++ )
		data->clut[i] = qt_screen->clut()[i];
	}
    }

    data->rw = qt_screen->mapToDevice( QSize(w,h) ).width();
    data->rh = qt_screen->mapToDevice( QSize(w,h) ).height();

    data->id=memorymanager->newPixmap(data->rw, data->rh, data->d, optim);
    if ( data->id == 0 )
	data->w = data->h = 0; // out of memory -- create null pixmap
}


void QPixmap::deref()
{
    if ( data && data->deref() ) {			// last reference lost
	if ( qws_trackPixmapData )
	    qws_pixmapData->removeRef( data );
	if ( data->mask )
	    delete data->mask;
	if ( data->clut )
	    delete[] data->clut;

	if ( memorymanager )
	    memorymanager->deletePixmap(data->id);
	delete data;
	data = 0;
    }
}


static void copyPixmapData(const uchar *src, int srcLinestep, uchar *dst, int dstLinestep, int height)
{
    int lineSize = QMIN(dstLinestep, srcLinestep);
    for ( int i = 0; i < height; i++ ) {
        memcpy(dst, src, lineSize);
        dst += dstLinestep;
        src += srcLinestep;
    }
}


void QPixmap::QPixmapData::copyData(int dstId, int width, int depth) const
{
    uchar *src = memorymanager->pixmapData(id);
    uchar *dst = memorymanager->pixmapData(dstId);
    int srcLinestep = memorymanager->pixmapLinestep(id,rw,d);
    int dstLinestep = memorymanager->pixmapLinestep(dstId,width,depth);
    copyPixmapData(src,srcLinestep,dst,dstLinestep,rh);
}


QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPaintDevice( QInternal::Pixmap )
{						// for bitmaps only
    init( 0, 0, 0, FALSE, defOptim );
    if ( w <= 0 || h <= 0 )			// create null pixmap
	return;

    data->uninit = FALSE;
    data->w = w;
    data->h = h;
    data->d = 1;
    data->rw = qt_screen->mapToDevice( QSize(w,h) ).width();
    data->rh = qt_screen->mapToDevice( QSize(w,h) ).height();
    data->hasAlpha = FALSE;
    uchar *flipped_bits;
    if ( isXbitmap ) {
	flipped_bits = 0;
    } else {					// not X bitmap -> flip bits
	flipped_bits = flip_bits( bits, ((w+7)/8)*h );
	bits = flipped_bits;
    }

    if ( qt_screen->isTransformed() ) {
	int bpl = isXbitmap ? (w+7)/8 : ((w+31)/32)*4;
	QImage img( (uchar *)bits, w, h, 1, bpl, 0, 0, QImage::LittleEndian );
	convertFromImage( img, MonoOnly );
	if ( flipped_bits )
	    delete [] flipped_bits;
	return;
    }

    data->id=memorymanager->newPixmap(data->rw,data->rh,data->d, optimization());
    if ( data->id == 0 ) {
	// out of memory -- create null pixmap.
	data->w = data->h = 0;
	return;
    }
    int linestep = memorymanager->pixmapLinestep(data->id,data->rw,data->d);
    copyPixmapData(bits,(w+7)/8,memorymanager->pixmapData(data->id),linestep,h);

    if ( flipped_bits )				// Avoid purify complaint
	delete [] flipped_bits;
}


void QPixmap::detach()
{
    if ( data->uninit || data->count == 1 )
	data->uninit = FALSE;
    else
	*this = copy();
}


int QPixmap::defaultDepth()
{
    QWSDisplay *d = qwsDisplay();
    int dd = d ? d->pixmapDepth() : 16;
    return dd;
}


void QPixmap::setOptimization( Optimization )
{
}


void QPixmap::fill( const QColor &fillColor )
{
    if ( isNull() )
	return;
    QPainter p(this);
    p.fillRect(rect(),fillColor);
}


int QPixmap::metric( int m ) const
{
    int val;
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
	val = width();
    } else if ( m == QPaintDeviceMetrics::PdmWidthMM ) {
	// 75 dpi is 3dpmm
	val = (width()*100)/288;
    } else if ( m == QPaintDeviceMetrics::PdmHeight ) {
	val = height();
    } else if ( m == QPaintDeviceMetrics::PdmHeightMM ) {
	val = (height()*100)/288;
    } else if ( m == QPaintDeviceMetrics::PdmDpiX ) {
	return 72;
    } else if ( m == QPaintDeviceMetrics::PdmDpiY ) {
	return 72;
    } else if( m ==  QPaintDeviceMetrics::PdmDepth ) {
	val=depth();
    } else {
	// XXX
	val = QPaintDevice::metric(m);
    }
    return val;
}

QImage QPixmap::convertToImage() const
{
    QImage image;
    if ( isNull() ) {
#if defined(CHECK_NULL)
	qWarning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#if defined(NASTY)
	abort();
#endif
#endif
	return image;
    }

    int w  = qt_screen->mapToDevice( QSize(width(), height()) ).width();
    int h  = qt_screen->mapToDevice( QSize(width(), height()) ).height();
    int	d  = depth();
    bool mono = d == 1;

    const QBitmap* msk = mask();

    if( d == 15 || d == 16 || d == 32 ) {
#ifdef QT_NO_QWS_DEPTH_16
	if( d == 32 )
#endif
	{
	    // Convert here because we may not have a 32bpp gfx
	    image.create( w,h,32,0, QImage::IgnoreEndian );

	    uchar *mskData = (msk) ? memorymanager->pixmapData(msk->data->id) : 0;
	    int mskLinestep = (msk) ? mskLinestep = msk->bytesPerLine() : 0;
	    uchar *pixData = memorymanager->pixmapData(data->id);
	    int pixLinestep = bytesPerLine();

	    ushort *s16 = 0;
	    uint *s32 = 0;
	    for ( int y=0; y < h; y++ ) {     // for each scan line...
		register uint *p = (uint *)image.scanLine(y);
		if( d == 32 )
		    s32 = (uint*)pixData;
		else
		    s16 = (ushort*)pixData;
		uint *end = p + w;
		if ( msk ) {
		    uchar* a = mskData;
		    uchar bit = 1; // mask is LittleEndian
		    uint rgb = 0;
		    if( d == 32 ) {
			while ( p < end ) {
			    rgb = *s32++;
			    if ( !(*a & bit) )
				rgb &= 0x00ffffff;
			    *p++ = rgb;
			    if (!(bit <<= 1)) {
				++a;
				bit = 1;
			    }
			}
		    } else {
			while ( p < end ) {
			    rgb = qt_conv16ToRgb( *s16++ );
			    if ( !(*a & bit) )
				rgb &= 0x00ffffff;
			    *p++ = rgb;
			    if (!(bit <<= 1)) {
				++a;
				bit = 1;
			    }
			}
		    }
		    mskData += mskLinestep;
		} else {
		    if( d == 32 ) {
			while( p < end )
			    *p++ = *s32++;
		    } else {
			while ( p < end )
			    *p++ = qt_conv16ToRgb( *s16++ );
		    }
		}
		pixData += pixLinestep;
	    }
	    if ( msk || d == 32 )
		image.setAlphaBuffer( TRUE );
	    d = 32;
	}
    } else {
	// We can only create little-endian pixmaps
	if ( d == 4 )
	    image.create(w,h,8,0, QImage::IgnoreEndian );
	else if ( d == 24 )
	    image.create(w,h,32,0, QImage::IgnoreEndian );
	else
	    image.create(w,h,d,0, mono ? QImage::LittleEndian : QImage::IgnoreEndian );//####### endianness

	QGfx * mygfx=image.graphicsContext();
	if(mygfx) {
	    mygfx->setSource(this);
	    mygfx->setAlphaType(QGfx::IgnoreAlpha);
	    mygfx->setLineStep(image.bytesPerLine());
	    mygfx->blt(0,0,width(),height(),0,0);
	} else {
	    qWarning("No image gfx for convertToImage!");
	}
	delete mygfx;
	image.setAlphaBuffer(data->hasAlpha);
    }

    if ( mono ) {				// bitmap
	image.setNumColors( 2 );
	image.setColor( 0, qRgb(255,255,255) );
	image.setColor( 1, qRgb(0,0,0) );
    } else if ( d <= 8 ) {
	image.setNumColors( numCols() );
	for ( int i = 0; i < numCols(); i++ )
	    image.setColor( i, clut()[i] );
    }

    image = qt_screen->mapFromDevice( image );

    return image;
}

bool QPixmap::convertFromImage( const QImage &img, int conversion_flags )
{
    if ( img.isNull() ) {
#if defined(CHECK_NULL)
	qWarning( "QPixmap::convertFromImage: Cannot convert a null image" );
#if defined(NASTY)
	abort();
#endif
#endif
	return FALSE;
    }

    QImage  image = img;
    int	 w   = image.width();
    int	 h   = image.height();
    int	 d   = image.depth();	// source depth
    int	 dd  = defaultDepth();	//destination depth
    bool force_mono = (dd == 1 || isQBitmap() ||
		       (conversion_flags & ColorMode_Mask)==MonoOnly );

    if ( force_mono ) {				// must be monochrome
	if ( d != 1 ) {
	    image = image.convertDepth( 1, conversion_flags );	// dither
	    d = 1;
	}
    } else {					// can be both
	bool conv8 = FALSE;
	if ( d > 8 && dd <= 8 ) {		// convert to 8 bit
	    if ( (conversion_flags & DitherMode_Mask) == AutoDither )
		conversion_flags = (conversion_flags & ~DitherMode_Mask)
					| PreferDither;
	    conv8 = TRUE;
	} else if ( (conversion_flags & ColorMode_Mask) == ColorOnly ) {
	    conv8 = d == 1;			// native depth wanted
	} else if ( d == 1 ) {
	    if ( image.numColors() == 2 ) {
		QRgb c0 = image.color(0);	// Auto: convert to best
		QRgb c1 = image.color(1);
		conv8 = QMIN(c0,c1) != qRgb(0,0,0) || QMAX(c0,c1) != qRgb(255,255,255);
		if ( !conv8 )
		    force_mono = TRUE;
	    } else {
		// eg. 1-color monochrome images (they do exist).
		conv8 = TRUE;
	    }
	}
	if ( conv8 ) {
	    image = image.convertDepth( 8, conversion_flags );
	    d = 8;
	}
    }

    if(image.depth()==1) {
	if(image.bitOrder()==QImage::BigEndian) {
	    image=image.convertBitOrder(QImage::LittleEndian);
	}
    }

    if ( force_mono )
	dd = 1;

    bool manycolors=(qt_screen->depth() > 8);

    bool partialalpha=FALSE;

    QWSDisplay *dpy = qwsDisplay();
    if(image.hasAlphaBuffer() && dpy->supportsDepth(32) && image.depth()==32 && dd>8 && manycolors) {
	int loopc,loopc2;
	for(loopc=0;loopc<image.height();loopc++) {
	    QRgb * tmp=(QRgb *)image.scanLine(loopc);
	    for(loopc2=0;loopc2<image.width();loopc2++) {
		int t=qAlpha(*tmp++);
		if(t>0 && t<255) {
		    partialalpha=TRUE;
		    loopc2=image.width();
		    loopc=image.height();
		}
	    }
	}
    }

    if ( partialalpha ) {
    	dd=32;
    }

    // for drivers that do not accelerate alpha blt
    Optimization optim = partialalpha ? NoOptim : defOptim;

    QImage rimg = qt_screen->mapToDevice( image );

    // detach other references and re-init()
    bool ibm = isQBitmap();
    detach();
    deref();
    init( w, h, dd, ibm, optim );

    QGfx * mygfx=graphicsContext();
    if ( mygfx ) {
	mygfx->setAlphaType(QGfx::IgnoreAlpha);
	if (image.depth() == 1) {
	    mygfx->setBrush(color0);
	    mygfx->setPen(color1);
	}
	mygfx->setSource(&rimg);
	mygfx->blt(0,0,data->w,data->h,0,0);
	delete mygfx;
    }
    if ( image.hasAlphaBuffer() ) {
	if ( !partialalpha ) {
	    QBitmap m;
	    m = image.createAlphaMask( conversion_flags );
	    setMask( m );
        } else
	    data->hasAlpha = TRUE;
    }
    data->uninit = FALSE;

    return TRUE;
}


QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    QPixmap pm;
    QWidget *widget = QWidget::find( window );
    if ( widget ) {
	if ( w <= 0 || h <= 0 ) {
	    if ( w == 0 || h == 0 )
		return pm;
	    if ( w < 0 )
		w = widget->width() - x;
	    if ( h < 0 )
		h = widget->height() - y;
	}
	pm.resize(w, h);
	QGfx *gfx=pm.graphicsContext();
	if ( gfx ) {
	    gfx->setAlphaType(QGfx::IgnoreAlpha);
	    gfx->setSource(widget);
	    gfx->blt(0,0,w,h,x,y);
	    delete gfx;
	}
    }
    return pm;
}

#ifndef QT_NO_PIXMAP_TRANSFORMATION
QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
    int	   w, h;				// size of target pixmap
    int	   ws, hs;				// size of source pixmap
    uchar *dptr;				// data in target pixmap
    int	   dbpl, dbytes;			// bytes per line/bytes total
    uchar *sptr;				// data in original pixmap
    int	   sbpl;				// bytes per line in original
    int	   bpp;					// bits per pixel
    bool   depth1 = depth() == 1;
    int	   y;

    if ( isNull() )				// this is a null pixmap
	return copy();

    ws = width();
    hs = height();

    const double dt = 0.0001;
    double x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    double xx = (double)ws - 1;
    double yy = (double)hs - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    double ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    double xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;

    // calculate new width and height
    QPointArray a( QRect(0,0,ws,hs) );
    a = mat.map( a );
    QRect r = a.boundingRect().normalize();
    w = r.width();
    h = r.height();

    if ( matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
	 matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F &&
	 depth() == defaultDepth() && // ### stretchBlt limitation
	 !data->mask
	)
    {
	if ( mat.m11() == 1.0F && mat.m22() == 1.0F )
	    return *this;			// identity matrix

	if(w==0 || h==0) {
	    return *this;
	}

	QPixmap pm( w, h, depth(), NormalOptim );
	QGfx * mygfx=pm.graphicsContext();
	if ( mygfx ) {
	    mygfx->setSource(this);
	    mygfx->setAlphaType(QGfx::IgnoreAlpha);
	    mygfx->stretchBlt(0,0,w,h,ws,hs);
	    delete mygfx;
	}
	if ( data->mask ) {
	    QBitmap bm =
		data->selfmask ? *((QBitmap*)(&pm)) :
					 data->mask->xForm(matrix);
	    pm.setMask( bm );
	}
	return pm;

    }
    bool invertible;
    mat = mat.invert( &invertible );		// invert matrix

    if ( h == 0 || w == 0 || !invertible ) {	// error, return null pixmap
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

    QImage srcImg;
    if ( qt_screen->isTransformed() ) {
	srcImg = convertToImage();
	sptr=srcImg.scanLine(0);
	sbpl=srcImg.bytesPerLine();
    } else {
	sptr=scanLine(0);
	sbpl=bytesPerLine();
    }
    ws=width();
    hs=height();

    QImage destImg;
    QPixmap pm( 1, 1, depth(), data->bitmap, NormalOptim );
    pm.data->uninit = FALSE;
    if ( qt_screen->isTransformed() ) {
	destImg.create( w, h, srcImg.depth(), srcImg.numColors(), srcImg.bitOrder() );
	dptr=destImg.scanLine(0);
	dbpl=destImg.bytesPerLine();
	bpp=destImg.depth();
    } else {
	pm.resize( w, h );
	dptr=pm.scanLine(0);
	dbpl=pm.bytesPerLine();
	bpp=pm.depth();
    }

    dbytes = dbpl*h;

    if ( depth1 )
	memset( dptr, 0x00, dbytes );
    else if ( bpp == 8 )
	memset( dptr, white.pixel(), dbytes );
    else if ( bpp == 32 ) {
	if ( qt_screen->isTransformed() )
	    destImg.fill( 0x00FFFFFF );
	else
	    pm.fill( 0x00FFFFFF );
    } else
	memset( dptr, 0xff, dbytes );

    int m11 = qRound((double)mat.m11()*65536.0);
    int m12 = qRound((double)mat.m12()*65536.0);
    int m21 = qRound((double)mat.m21()*65536.0);
    int m22 = qRound((double)mat.m22()*65536.0);
    int dx  = qRound((double)mat.dx() *65536.0);
    int dy  = qRound((double)mat.dy() *65536.0);

    int	  m21ydx = dx, m22ydy = dy;
    uint  trigx, trigy;
    uint  maxws = ws<<16, maxhs=hs<<16;
    uchar *p	= dptr;
    int	  xbpl, p_inc;

    if ( depth1 ) {
	xbpl  = (w+7)/8;
	p_inc = dbpl - xbpl;
    } else {
	xbpl  = (w*bpp)/8;
	p_inc = dbpl - xbpl;
    }

    for ( y=0; y<h; y++ ) {			// for each target scanline
	trigx = m21ydx;
	trigy = m22ydy;
	uchar *maxp = p + xbpl;
	if ( !depth1 ) {
	    switch ( bpp ) {
		case 8:				// 8 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*p = *(sptr+sbpl*(trigy>>16)+(trigx>>16));
		    trigx += m11;
		    trigy += m12;
		    p++;
		}
		break;

		case 16:			// 16 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((ushort*)p) = *((ushort *)(sptr+sbpl*(trigy>>16) +
						     ((trigx>>16)<<1)));
		    trigx += m11;
		    trigy += m12;
		    p++;
		    p++;
		}
		break;

		case 24: {			// 24 bpp transform
		uchar *p2;
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs ) {
			p2 = sptr+sbpl*(trigy>>16) + ((trigx>>16)*3);
			p[0] = p2[0];
			p[1] = p2[1];
			p[2] = p2[2];
		    }
		    trigx += m11;
		    trigy += m12;
		    p += 3;
		}
		}
		break;

		case 32:			// 32 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((uint*)p) = *((uint *)(sptr+sbpl*(trigy>>16) +
						   ((trigx>>16)<<2)));
		    trigx += m11;
		    trigy += m12;
		    p += 4;
		}
		break;

		default: {
#if defined(CHECK_RANGE)
		qWarning( "QPixmap::xForm: Display not supported (bpp=%d)",bpp);
#endif
		return QPixmap( 0, 0, 0, data->bitmap, data->optim );
		}
	    }
	} else {
	    // mono bitmap LSB first
	    while ( p < maxp ) {
#undef IWX
#define IWX(b)  if ( trigx < maxws && trigy < maxhs ) {                       \
		    if ( *(sptr+sbpl*(trigy>>16)+(trigx>>19)) &               \
			 (1 << ((trigx>>16)&7)) )                             \
			*p |= b;                                              \
		}                                                             \
		trigx += m11;                                                 \
		trigy += m12;
		// END OF MACRO
		IWX(1)
		IWX(2)
		IWX(4)
		IWX(8)
		IWX(16)
		IWX(32)
		IWX(64)
		IWX(128)
		p++;
	    }
	}
	m21ydx += m21;
	m22ydy += m22;
	p += p_inc;
    }

    if ( qt_screen->isTransformed() ) {
	pm.convertFromImage( destImg );
    }

    if ( depth1 ) {
	if ( data->mask ) {
	    if ( data->selfmask )               // pixmap == mask
		pm.setMask( *((QBitmap*)(&pm)) );
	    else
		pm.setMask( data->mask->xForm(matrix) );
	}
    } else {
	if ( data->mask )
	    pm.setMask( data->mask->xForm(matrix) );
    }

    return pm;
}
#endif // QT_NO_PIXMAP_TRANSFORMATION


#ifndef QT_NO_WMATRIX
QWMatrix QPixmap::trueMatrix( const QWMatrix &matrix, int w, int h )
{
    const double dt = (double)0.0001;
    double x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    double xx = (double)w - 1;
    double yy = (double)h - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    double ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    double xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;
    return mat;
}
#endif // QT_NO_WMATRIX

// CALLER DELETES
QGfx * QPixmap::graphicsContext(bool) const
{
    if(isNull()) {
	qDebug("Can't make QGfx for null pixmap\n");
	return 0;
    }
    uchar *mydata = memorymanager->pixmapData(data->id);
    QGfx *ret = QGfx::createGfx(depth(), mydata, data->w, data->h, bytesPerLine());
    if(data->d<=8) {
	if(data->d==1 && !(data->clut)) {
	    data->clut=new QRgb[2];
	    data->clut[0]=qRgb(255,255,255);
	    data->clut[1]=qRgb(0,0,0);
	    data->numcols = 2;
	}
	if ( data->numcols )
	    ret->setClut(data->clut,data->numcols);
    }
    return ret;
}

unsigned char * QPixmap::scanLine(int i) const
{
    return memorymanager->pixmapData(data->id) + i * bytesPerLine();
}

int QPixmap::bytesPerLine() const
{
    return memorymanager->pixmapLinestep(data->id,data->rw,data->d);
}

QRgb * QPixmap::clut() const
{
    return data->clut;
}

int QPixmap::numCols() const
{
    return data->numcols;
}
