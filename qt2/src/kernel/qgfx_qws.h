/****************************************************************************
** $Id: qt/src/kernel/qgfx_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of QGfx (graphics context) class
**
** Created : 940721
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

#ifndef QGFX_H
#define QGFX_H

#ifndef QT_H
#include "qwidget.h"
#include "qnamespace.h"
#include "qimage.h"
#include "qfontmanager_qws.h"
#endif // QT_H


#if !defined( QT_NO_IMAGE_16_BIT ) || !defined( QT_NO_QWS_DEPTH_16 )
# ifndef QT_QWS_DEPTH16_RGB
#  define QT_QWS_DEPTH16_RGB 565
# endif
static const int qt_rbits = (QT_QWS_DEPTH16_RGB/100);
static const int qt_gbits = (QT_QWS_DEPTH16_RGB/10%10);
static const int qt_bbits = (QT_QWS_DEPTH16_RGB%10);
static const int qt_red_shift = qt_bbits+qt_gbits-(8-qt_rbits);
static const int qt_green_shift = qt_bbits-(8-qt_gbits);
static const int qt_neg_blue_shift = 8-qt_bbits;
static const int qt_blue_mask = (1<<qt_bbits)-1;
static const int qt_green_mask = (1<<(qt_gbits+qt_bbits))-(1<<qt_bbits);
static const int qt_red_mask = (1<<(qt_rbits+qt_gbits+qt_bbits))-(1<<(qt_gbits+qt_bbits));

inline ushort qt_convRgbTo16( const int r, const int g, const int b )
{
    const int tr = r << qt_red_shift;
    const int tg = g << qt_green_shift;
    const int tb = b >> qt_neg_blue_shift;

    return (tb & qt_blue_mask) | (tg & qt_green_mask) | (tr & qt_red_mask);
}

inline ushort qt_convRgbTo16( QRgb c )
{
    const int tr = qRed(c) << qt_red_shift;
    const int tg = qGreen(c) << qt_green_shift;
    const int tb = qBlue(c) >> qt_neg_blue_shift;

    return (tb & qt_blue_mask) | (tg & qt_green_mask) | (tr & qt_red_mask);
}

inline QRgb qt_conv16ToRgb( ushort c )
{
    const int r=(c & qt_red_mask);
    const int g=(c & qt_green_mask);
    const int b=(c & qt_blue_mask);
    const int tr = r >> qt_red_shift;
    const int tg = g >> qt_green_shift;
    const int tb = b << qt_neg_blue_shift;

    return qRgb(tr,tg,tb);
}

inline void qt_conv16ToRgb( ushort c, int& r, int& g, int& b )
{
    const int tr=(c & qt_red_mask);
    const int tg=(c & qt_green_mask);
    const int tb=(c & qt_blue_mask);
    r = tr >> qt_red_shift;
    g = tg >> qt_green_shift;
    b = tb << qt_neg_blue_shift;
}
#endif


const int SourceSolid=0;
const int SourcePixmap=1;

#ifndef QT_NO_QWS_CURSOR

extern bool qt_sw_cursor;

class QGfxRasterBase;

#define SW_CURSOR_DATA_SIZE	4096  // 64x64 8-bit cursor

class SWCursorData;

class QScreenCursor
{
public:
    QScreenCursor( );
    virtual ~QScreenCursor();

    virtual void init(SWCursorData *da, bool init = FALSE);

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
    virtual void show();
    virtual void hide();

    virtual bool restoreUnder( const QRect &r, QGfxRasterBase *g = 0 );
    virtual void saveUnder();
    virtual void drawCursor();
    void draw();
    virtual bool supportsAlphaCursor();

    static bool enabled() { return qt_sw_cursor; }

protected:
    QGfxRasterBase *gfx;
    QGfxRasterBase *gfxunder;

    QImage *imgunder;
    const QImage *cursor;

    uchar *fb_start;
    uchar *fb_end;
    bool save_under;
    SWCursorData *data;

    int clipWidth;
    int clipHeight;
    int myoffset;

};

extern QScreenCursor * qt_screencursor;

#endif // QT_NO_QWS_CURSOR

struct fb_cmap;

// A (used) chunk of offscreen memory

class QPoolEntry
{
public:
    unsigned int start;
    unsigned int end;
    int clientId;
};


class QScreen;
typedef void(*ClearCacheFunc)(QScreen *obj, int);

class QScreen {

public:

    QScreen( int display_id );
    virtual ~QScreen();
    virtual bool initDevice() = 0;
    virtual bool connect( const QString &displaySpec ) = 0;
    virtual void disconnect() = 0;
    virtual int initCursor(void *, bool=FALSE);
    virtual void shutdownDevice();
    virtual void setMode(int,int,int) = 0;
    virtual bool supportsDepth(int) const;
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual QGfx * screenGfx();
    virtual void save();
    virtual void restore();
    virtual void blank(bool on);

    virtual int pixmapOffsetAlignment() { return 64; }
    virtual int pixmapLinestepAlignment() { return 64; }

    virtual bool onCard(unsigned char *) const;
    virtual bool onCard(unsigned char *, ulong& out_offset) const;

    // sets a single color in the colormap
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    // allocates a color
    virtual int alloc(unsigned int,unsigned int,unsigned int);

    int width() const { return w; }
    int height() const { return h; }
    int depth() const { return d; }
    virtual int pixmapDepth() const;
    int pixelType() const { return pixeltype; }
    int linestep() const { return lstep; }
    int deviceWidth() const { return dw; }
    int deviceHeight() const { return dh; }
    uchar * base() const { return data; }
    // Ask for memory from card cache with alignment
    virtual uchar * cache(int,int) { return 0; }
    virtual void uncache(uchar *) {}

    int screenSize() const { return size; }
    int totalSize() const { return mapsize; }

    QRgb * clut() { return screenclut; }
    int numCols() { return screencols; }

    virtual QSize mapToDevice( const QSize & ) const;
    virtual QSize mapFromDevice( const QSize & ) const;
    virtual QPoint mapToDevice( const QPoint &, const QSize & ) const;
    virtual QPoint mapFromDevice( const QPoint &, const QSize & ) const;
    virtual QRect mapToDevice( const QRect &, const QSize & ) const;
    virtual QRect mapFromDevice( const QRect &, const QSize & ) const;
    virtual QImage mapToDevice( const QImage & ) const;
    virtual QImage mapFromDevice( const QImage & ) const;
    virtual QRegion mapToDevice( const QRegion &, const QSize & ) const;
    virtual QRegion mapFromDevice( const QRegion &, const QSize & ) const;
    virtual int transformOrientation() const;
    virtual bool isTransformed() const;

    virtual bool isInterlaced() const;

protected:

    QRgb screenclut[256];
    int screencols;

    bool initted;

    uchar * data;

    // Table of allocated lumps, kept in sorted highest-to-lowest order
    // The table itself is allocated at the bottom of offscreen memory
    // i.e. it's similar to having a stack (the table) and a heap
    // (the allocated blocks). Freed space is implicitly described
    // by the gaps between the allocated lumps (this saves entries and
    // means we don't need to worry about coalescing freed lumps)

    QPoolEntry * entries;
    int * entryp;
    unsigned int * lowest;

    int w;
    int lstep;
    int h;
    int d;
    int pixeltype;
    
    int dw;
    int dh;

    int hotx;
    int hoty;
    QImage cursor;

    int size;	       // Screen size
    int mapsize;       // Total mapped memory

    int displayId;

    friend class QWSServer;
    static ClearCacheFunc clearCacheFunc;
};

extern QScreen * qt_screen;

class Q_EXPORT QGfx : public Qt {
public:
    // With loadable drivers, do probe here
    static QGfx *createGfx( int depth, unsigned char *buffer,
			    int w, int h, int linestep );

    virtual ~QGfx() {}

    virtual void setPen( const QPen & )=0;
    virtual void setFont( const QFont & )=0;
    virtual void setBrush( const QBrush & )=0;
    virtual void setBrushPixmap( const QPixmap * )=0;
    virtual void setBrushOffset( int, int ) = 0;
    virtual void setClipRect( int,int,int,int )=0;
    virtual void setClipRegion( const QRegion & )=0;
    virtual void setClipDeviceRegion( const QRegion & )=0;
    virtual void setClipping (bool)=0;
    // These will be called from qwidget_qws or qwidget_mac
    // to update the drawing area when a widget is moved
    virtual void setOffset( int,int )=0;
    virtual void setWidgetRect( int,int,int,int )=0;
    virtual void setWidgetRegion( const QRegion & )=0;
    virtual void setWidgetDeviceRegion( const QRegion & )=0;
    virtual void setSourceWidgetOffset(int x, int y) = 0;
    virtual void setGlobalRegionIndex( int idx ) = 0;

    virtual void setDashedLines(bool d) = 0;
    virtual void setDashes(char *, int) = 0;

    virtual void setOpaqueBackground(bool b)=0;
    virtual void setBackgroundColor(QColor c)=0;

    // Drawing operations
    virtual void drawPoint( int,int )=0;
    virtual void drawPoints( const QPointArray &,int,int )=0;
    virtual void moveTo( int,int )=0;
    virtual void lineTo( int,int )=0;
    virtual void drawLine( int,int,int,int )=0;
    virtual void drawPolyline( const QPointArray &,int,int )=0;

    // Fill operations - these use the current source (pixmap,
    // color, etc), and draws outline
    virtual void fillRect( int,int,int,int )=0;
    virtual void drawPolygon( const QPointArray &,bool,int,int )=0;

    virtual void setLineStep(int)=0;

    // Special case of rect-with-pixmap-fill for speed/hardware acceleration
    virtual void blt( int,int,int,int,int,int )=0;
    virtual void scroll( int,int,int,int,int,int )=0;

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS) || !defined(QT_NO_PIXMAP_TRANSFORMATION)
    virtual void stretchBlt( int,int,int,int,int,int )=0;
#endif
    virtual void tiledBlt( int,int,int,int )=0;

    enum SourceType { SourcePen, SourceImage, SourceAccel };
    enum PixelType { NormalPixel, BGRPixel };
    
    // Setting up source data - can be solid color or pixmap data
    virtual void setSource(const QPaintDevice *)=0;
    virtual void setSource(const QImage *)=0;
    // This one is pen
    virtual void setSourcePen()=0;

    virtual void drawAlpha(int,int,int,int,int,int,int,int) {}

    virtual void hsync(int) {}

    // These apply only to blt's. For alpha values for general
    // drawing operations we should probably have a separate QGfx
    // class. It's not a high priority though.

    // Enum values: Ignore alpha information, alpha information encoded in
    // 32-bit rgba along with colors, alpha information in 8bpp
    // format in alphabits

    enum AlphaType { IgnoreAlpha, InlineAlpha, SeparateAlpha,
                     LittleEndianMask, BigEndianMask, SolidAlpha };

    // Can be no alpha, inline (32bit data), separate (for images),
    // LittleEndianMask/BigEndianMask 1bpp masks, constant alpha
    // value
    virtual void setAlphaType(AlphaType)=0;
    // Pointer to data, linestep
    virtual void setAlphaSource(unsigned char *,int)=0;
    virtual void setAlphaSource(int,int=-1,int=-1,int=-1)=0;

    virtual void drawText( int,int,const QString & )=0;

    virtual void setClut(QRgb *,int)=0;

    // Save and restore pen and brush state - necessary when setting
    // up a bitBlt for example
    virtual void save()=0;
    virtual void restore()=0;

    virtual void setRop(RasterOp)=0;

    bool isScreenGfx() { return is_screen_gfx; } //for cursor..
    
protected:
    bool is_screen_gfx;
};


// This lives in loadable modules

#ifndef QT_LOADABLE_MODULES
extern "C" QScreen * qt_get_screen( int display_id, const char* spec );
#endif

// This is in main lib, loads the right module, calls qt_get_screen
// In non-loadable cases just aliases to qt_get_screen

const unsigned char * qt_probe_bus();

#endif




