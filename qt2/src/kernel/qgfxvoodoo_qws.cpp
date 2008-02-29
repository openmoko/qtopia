/****************************************************************************
** $Id: qt/src/kernel/qgfxvoodoo_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QGfxVoodoo (graphics context) class for Voodoo 3 cards
**
** Created : 000503
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

#include "qgfxvoodoodefs_qws.h"

#ifndef QT_NO_QWS_VOODOO3

#include "qgfxlinuxfb_qws.h"

#include "qimage.h"

// This is intended as an example of an accelerated driver, as is
// Mach64 driver. The Mach64 driver implements more functionality;
// this is a simpler driver which may be easier to follow. This driver
// should work with 3dfx Voodoo 3 and Voodoo Banshee cards (PCI or AGP)
// Note that the register-level specifications for these cards are freely
// available from the 3dfx web site

// Pointer to Voodoo 3 registers
static unsigned char *voodoo_regbase=0;

//#define DEBUG_INIT

// An integer, lastop, is stored in shared memory and is set to one
// of these values. The reason for this is that if an accelerated
// LASTOP_RECT is followed by another LASTOP_RECT, for example,
// some register setup can be avoided

#define LASTOP_LINE 1
#define LASTOP_RECT 2
#define LASTOP_BLT 3
#define LASTOP_BLTPEN 4
#define LASTOP_STRETCHBLT 5
#define LASTOP_RESET 6
#define LASTOP_POLYGON 7
#define LASTOP_TEXT 8
#define LASTOP_ALPHA 9
#define LASTOP_TILEDBLT 10
#define LASTOP_TILEDBLTPEN 11
#define LASTOP_SYNC 12

// Read a 32-bit graphics card register from 2d engine register block
inline unsigned int voodoo_regr(volatile unsigned int regindex)
{
    unsigned long int val;
    val=*((volatile unsigned long *)(voodoo_regbase+regindex));
    return val;
}

// Write a 32-bit graphics card register to 2d engine register block
inline void voodoo_regw(volatile unsigned int regindex,unsigned long val)
{
    *((volatile unsigned long int *)(voodoo_regbase+regindex))=val;
}

// Wait <entry> FIFO entries. <entry> FIFO entries must be free
// before making <entry> regw's or regw2's, or you'll lock up the
// graphics card and your computer. The total number of FIFO entries
// varies from card to card.

inline void voodoo_wait_for_fifo(short entries)
{
    int trycount=0;

    while(trycount++) {
	int fifoval=voodoo_regr(VOODOOSTATUS);
	fifoval=fifoval & 0x1f;
	if(fifoval>=entries) {
	    return;
	}
    }
}

template <const int depth, const int type>
class QGfxVoodoo : public QGfxRaster<depth,type> {

public:

    QGfxVoodoo(unsigned char *,int w,int h);

    virtual void fillRect(int,int,int,int);
    virtual void blt(int,int,int,int,int,int);
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt(int,int,int,int,int,int);
#endif
    virtual void drawLine(int,int,int,int);
    virtual void sync();

private:

    bool checkSourceDest();
    bool checkDest();
    void do_scissors(QRect &);

};

template<const int depth,const int type>
inline void QGfxVoodoo<depth,type>::do_scissors(QRect & r)
{
    voodoo_wait_for_fifo(2);
    voodoo_regw(CLIP0MIN,(r.top()) << 16 | r.left());
    voodoo_regw(CLIP0MAX,(r.bottom()+1) << 16 | (r.right()+1));
}

template<const int depth,const int type>
inline void QGfxVoodoo<depth,type>::sync()
{
    // NOP to avoid documented deadlock
    (*lastop)=LASTOP_SYNC;
    voodoo_regw(COMMAND,0x100);

    // Now wait until we're told graphics engine is idle
    int loopc;
    for(loopc=0;loopc<1000;loopc++) {
        unsigned int stat=voodoo_regr(VOODOOSTATUS);
	if((!(stat & 0x300)) && loopc>3) {
	    return;
	}
    }
    qDebug("Idle timeout!");
}

// Figures out the 3-bit code for different depths
inline int voodoo_depthcode(int d)
{
    int ret;
    if(d==32) {
	ret=5;
    } else if(d==16) {
	ret=3;
    } else if(d==8) {
	ret=1;
    } else {
	qFatal("Unexpected depth %d",d);
	ret=0;
    }
    return ret;
}

// Set up DST_OFF_PITCH, return false if it's not on the card
// For lines, filled rects etc
// This is similar to checkSourceDest but is used when there is no
// source image data
template<const int depth,const int type>
inline bool QGfxVoodoo<depth,type>::checkDest()
{
    ulong buffer_offset;
    if (!qt_screen->onCard(this->buffer,buffer_offset)) {
	return FALSE;
    }

    voodoo_wait_for_fifo(4);
    voodoo_regw(DSTBASEADDR,buffer_offset);
    voodoo_regw(DSTFORMAT,this->linestep() | (voodoo_depthcode(depth) << 16));
    voodoo_regw(CLIP0MIN,0);
    voodoo_regw(CLIP0MAX,(this->height << 16) | this->width);

    return TRUE;
}

// Sets up the graphics engine's idea of bits-per-pixel for destination
// and source, used for, example, blt's
template<const int depth,const int type>
inline bool QGfxVoodoo<depth,type>::checkSourceDest()
{
    if ( !checkDest() ) {
	return FALSE;
    }

    ulong src_buffer_offset;
    if (this->srctype == QGfx::SourcePen) {
	src_buffer_offset = -1;
    } else {
	if (!qt_screen->onCard(this->srcbits,src_buffer_offset)) {
	    return FALSE;
	}
	if(src_buffer_offset & 0x7) {
	    qDebug("Unaligned offset %lx",src_buffer_offset);
	    return FALSE;
	}
	int srcstep;
	if (this->srclinestep==0) {
	    srcstep=(this->width*this->srcdepth)/8;
	} else {
	    srcstep=this->srclinestep;
	}

	voodoo_wait_for_fifo(2);
	voodoo_regw(SRCBASEADDR, src_buffer_offset);
	voodoo_regw(SRCFORMAT,(srcstep | (voodoo_depthcode(this->srcdepth) << 16)));
    }
    return TRUE;
}

template<const int depth,const int type>
QGfxVoodoo<depth,type>::QGfxVoodoo(unsigned char * a,int b,int c)
    : QGfxRaster<depth,type>(a,b,c)
{
}

template<const int depth,const int type>
void QGfxVoodoo<depth,type>::fillRect(int rx,int ry,int w,int h)
{
    // No point going any further if the window isn't visible
    if(this->ncliprect<1) {
	return;
    }

    // Only handle 'norma' rectangles
    if( (this->cbrush.style()!=this->NoBrush) && (this->cbrush.style()!=this->SolidPattern) ) {
	QGfxRaster<depth,type>::fillRect(rx,ry,w,h);
	return;
    }

    // Stop anyone else trying to access optype/lastop/the graphics engine
    // to avoid synchronization problems with other processes
    QWSDisplay::grab( TRUE );
    if(!checkDest()) {
	QWSDisplay::ungrab();
	QGfxRaster<depth,type>::fillRect(rx,ry,w,h);
	return;
    }

    // This is used by the software mouse cursor to prevent corruption
    // of the cursor if a drawing operation is performed under it.
    // GFX_START/END can be omitted if you know you'll only ever use
    // the hardware cursor

    GFX_START(QRect(rx+this->xoffs, ry+this->yoffs, w+1, h+1))

    int loopc;

    int xp=rx+this->xoffs;
    int yp=ry+this->yoffs;

    int x2,y2;

    x2=xp+(w-1);
    y2=yp+(h-1);

    int x3,y3,x4,y4;

    if((*lastop)!=LASTOP_RECT) {
	voodoo_wait_for_fifo(2);
	voodoo_regw(SRCFORMAT,3 << 16);
	// With the Voodoo 3 you write the command code into COMMAND
	// and then write parameters (usually x/y coordinates of some sort)
	// into LAUNCHAREA to kick off the operation
	voodoo_regw(COMMAND,0x5 | (0xcc << 24));
    }

    (*optype)=1;
    (*lastop)=LASTOP_RECT;

    voodoo_wait_for_fifo(1);
    voodoo_regw(COLORFORE,this->srccol);

    // We clip in software here because rectangle-rectangle intersections
    // are very fast, probably much more so than writing graphics card
    // registers to set up the clip

    if(this->cbrush.style()!=this->NoBrush) {
	int p=this->ncliprect;
	if(p<8) {
	    // We can wait for all our fifos at once
	    // (slight performance optimisation)
	    voodoo_wait_for_fifo(p*2);
	    for(loopc=0;loopc<p;loopc++) {
		QRect r=this->cliprect[loopc];
		// Clip rectangle to current clip rectangle
		if(xp<=r.right() && yp<=r.bottom() &&
		   x2>=r.left() && y2>=r.top()) {
		    x3=r.left() > xp ? r.left() : xp;
		    y3=r.top() > yp ? r.top() : yp;
		    x4=r.right() > x2 ? x2 : r.right();
		    y4=r.bottom() > y2 ? y2 : r.bottom();
		    int ww=(x4-x3)+1;
		    int hh=(y4-y3)+1;
		    voodoo_regw(DSTSIZE,(hh << 16) | ww);
		    voodoo_regw(LAUNCHAREA,x3 | (y3 << 16));
		}
	    }
	} else {
	    for(loopc=0;loopc<p;loopc++) {
		QRect r=this->cliprect[loopc];
		if(xp<=r.right() && yp<=r.bottom() &&
		   x2>=r.left() && y2>=r.top()) {
		    x3=r.left() > xp ? r.left() : xp;
		    y3=r.top() > yp ? r.top() : yp;
		    x4=r.right() > x2 ? x2 : r.right();
		    y4=r.bottom() > y2 ? y2 : r.bottom();
		    int ww=(x4-x3)+1;
		    int hh=(y4-y3)+1;
		    voodoo_wait_for_fifo(2);
		    voodoo_regw(DSTSIZE,(hh << 16) | ww);
		    voodoo_regw(LAUNCHAREA,x3 | (y3 << 16));
		}
	    }
	}
    }
    GFX_END
    QWSDisplay::ungrab();
}

template<const int depth,const int type>
inline void QGfxVoodoo<depth,type>::blt(int rx,int ry,int w,int h, int sx, int sy)
{
    if(this->ncliprect<1)
	return;

    if(this->srctype==QGfx::SourceImage && this->alphatype!=QGfx::IgnoreAlpha) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
	return;
    }

    if(this->srctype==QGfx::SourcePen) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
	return;
    }

    if( (this->srcdepth!=32) && (this->srcdepth!=16) && (this->srcdepth!=8) ) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
	return;
    }

    QWSDisplay::grab( TRUE );

    if(checkSourceDest()) {

	(*optype)=1;
	(*lastop)=LASTOP_BLT;

	int xp=this->xoffs+rx;
	int yp=this->yoffs+ry;
	int xp2=this->srcwidgetoffs.x() + sx;
	int yp2=this->srcwidgetoffs.y() + sy;

	QRect cursRect(xp, yp, w+1, h+1);

	GFX_START(cursRect)

        unsigned int dirmask=0;

	// Tell the engine whether to copy bits from left to right,
	// top to bottom, right to left, bottom to top - this is
	// important for getting the right results with an overlapping
	// blt

	if(yp>yp2) {
	    // Down, reverse
	    if(xp>xp2) {
		// Right, reverse
		xp+=(w-1);
		xp2+=(w-1);
		yp+=(h-1);
		yp2+=(h-1);
		dirmask|=0x4000 | 0x8000;
	    } else {
		// Left, normal
		yp+=(h-1);
		yp2+=(h-1);
		dirmask|=0x8000;
	    }
	} else {
	    // Up, normal
	    // Down, reverse
	    if(xp>xp2) {
		// Right, reverse
		xp+=(w-1);
		xp2+=(w-1);
		dirmask|=0x4000;
	    } else {
		// Left, normal
	    }
	}

	// Wait for vsync if screen-to-screen blt (to smooth
	// window movement)
	if(this->srcbits==this->buffer)
	    voodoo_regw(COMMANDEXTRA,0x4);

	voodoo_wait_for_fifo(4);
	voodoo_regw(COMMAND,0x1 | (0x1cc << 24) | dirmask);
	voodoo_regw(SRCSIZE,w | (h << 16));
	voodoo_regw(DSTSIZE,w | (h << 16));
	voodoo_regw(DSTXY,xp | (yp << 16));

	int loopc;
	for(loopc=0;loopc<this->ncliprect;loopc++) {
	    do_scissors(this->cliprect[loopc]);
	    voodoo_wait_for_fifo(1);
	    voodoo_regw(LAUNCHAREA,xp2 | (yp2 << 16));
	}
	voodoo_wait_for_fifo(5);
	voodoo_regw(CLIP0MIN,0);
	voodoo_regw(CLIP0MAX,(this->height << 16) | this->width);
	voodoo_regw(CLIP0MIN,0);
	voodoo_regw(CLIP0MAX,(this->height << 16) | this->width);
	voodoo_regw(COMMANDEXTRA,0);

	GFX_END
	QWSDisplay::ungrab();

	return;
    } else {
	QWSDisplay::ungrab();
	// software fallback
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
    }
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template<const int depth,const int type>
inline void QGfxVoodoo<depth,type>::stretchBlt(int rx,int ry,int w,int h,
					       int sw,int sh)
{
    // On Voodoo3's, as opposed to Mach64, stretchBlt is a normal 2d operation
    // and not part of a separate pipeline
    // Hence optype is set to 1, since a sequence like blt/stretchBlt/blt
    // is guaranteed to be performed in that order; on Mach64 without a
    // sync() before the second blt the stretchBlt and second blt might overlap
    // and both write to the same area simultaneously
 
    if(this->ncliprect)
	return;

    if(this->srctype==QGfx::SourceImage && this->alphatype!=QGfx::IgnoreAlpha) {
	QGfxRaster<depth,type>::stretchBlt(rx,ry,w,h,sw,sh);
	return;
    }

    if(this->srctype==QGfx::SourcePen) {
	QGfxRaster<depth,type>::stretchBlt(rx,ry,w,h,sw,sh);
	return;
    }

    if( (this->srcdepth!=32) && (this->srcdepth!=16) && (this->srcdepth!=8) ) {
	QGfxRaster<depth,type>::stretchBlt(rx,ry,w,h,sw,sh);
	return;
    }

    QWSDisplay::grab( TRUE );

    if(checkSourceDest()) {

	(*optype)=1;
	(*lastop)=LASTOP_STRETCHBLT;

	int xp=this->xoffs+rx;
	int yp=this->yoffs+ry;
	int xp2=this->srcwidgetoffs.x(); // + sx;
	int yp2=this->srcwidgetoffs.y(); // + sy;

	QRect cursRect(xp, yp, w+1, h+1);

	GFX_START(cursRect)

	voodoo_wait_for_fifo(4);
	voodoo_regw(COMMAND,0x2 | (0xcc << 24));
	voodoo_regw(SRCSIZE,sw | (sh << 16));
	voodoo_regw(DSTSIZE,w | (h << 16));
	voodoo_regw(DSTXY,xp | (yp << 16));

	int loopc;
	for(loopc=0;loopc<this->ncliprect;loopc++) {
	    do_scissors(this->cliprect[loopc]);
	    voodoo_wait_for_fifo(1);
	    voodoo_regw(LAUNCHAREA,xp2 | (yp2 << 16));
	}
	voodoo_wait_for_fifo(4);
	voodoo_regw(CLIP0MIN,0);
	voodoo_regw(CLIP0MAX,(this->height << 16) | this->width);
	voodoo_regw(CLIP0MIN,0);
	voodoo_regw(CLIP0MAX,(this->height << 16) | this->width);

	GFX_END
	QWSDisplay::ungrab();
	return;
    } else {
	QWSDisplay::ungrab();
	QGfxRaster<depth,type>::stretchBlt(rx,ry,w,h,sw,sh);
    }
}
#endif

template<const int depth,const int type>
void QGfxVoodoo<depth,type>::drawLine(int x1,int y1,int x2,int y2)
{
    if(this->ncliprect<1 || this->myrop!=this->CopyROP || this->cpen.style()!=this->SolidLine)
	return;

    QWSDisplay::grab( TRUE );

    if(checkDest()) {

	(*optype)=1;
	(*lastop)=LASTOP_LINE;

	x1+=this->xoffs;
	y1+=this->yoffs;
	x2+=this->xoffs;
	y2+=this->yoffs;

	int dx,dy;
	dx=abs(x2-x1);
	dy=abs(y2-y1);

	// On the Voodoo3, unlike the Mach64, Bresenham parameters
	// for the line are calculated automatically

        GFX_START(QRect(x1, y1 < y2 ? y1 : y2, dx+1, QABS(dy)+1))

	QColor tmp=this->cpen.color();
	unsigned int tmp2=tmp.alloc();

	int loopc;

	voodoo_wait_for_fifo(2);
	voodoo_regw(COLORFORE,tmp2);
	voodoo_regw(COMMAND,0x6 | (0xcc << 24));

	for(loopc=0;loopc<this->ncliprect;loopc++) {
	    do_scissors(this->cliprect[loopc]);
	    voodoo_wait_for_fifo(2);
	    voodoo_regw(SRCXY,x1 | (y1 << 16));
	    voodoo_regw(LAUNCHAREA,x2 | (y2 << 16));
	}
	voodoo_wait_for_fifo(4);
	voodoo_regw(CLIP0MIN,0);
	voodoo_regw(CLIP0MAX,(this->height << 16) | this->width);
	voodoo_regw(CLIP0MIN,0);
	voodoo_regw(CLIP0MAX,(this->height << 16) | this->width);

	GFX_END
	QWSDisplay::ungrab();
	return;
    } else {
	QWSDisplay::ungrab();
	QGfxRaster<depth,type>::drawLine(x1,y1,x2,y2);
    }
}

// This does card-specific setup and constructs accelerated gfx's and
// the accelerated cursor

class QVoodooScreen : public QLinuxFbScreen {

public:

    QVoodooScreen( int display_id );
    virtual ~QVoodooScreen();
    virtual bool connect( const QString &spec );
    virtual bool initDevice();
    virtual void shutdownDevice();
    virtual int initCursor(void *,bool);
    virtual bool useOffscreen() { return true; }

    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
};

#ifndef QT_NO_QWS_CURSOR
class QVoodooCursor : public QScreenCursor
{
public:
    QVoodooCursor();
    ~QVoodooCursor();

    virtual void init(SWCursorData *,bool=FALSE);

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
    virtual void show();
    virtual void hide();

    virtual bool restoreUnder( const QRect &, QGfxRasterBase * = 0 )
                { return FALSE; }
    virtual void saveUnder() {}
    virtual void drawCursor() {}
    virtual void draw() {}
    virtual bool supportsAlphaCursor() { return false; }

    static bool enabled() { return false; }

private:

    int hotx;
    int hoty;

};
#endif // QT_NO_QWS_CURSOR

QVoodooScreen::QVoodooScreen( int display_id  )
    : QLinuxFbScreen( display_id )
{
}

bool QVoodooScreen::connect( const QString &spec )
{
    if (!QLinuxFbScreen::connect( spec )) {
	return FALSE;
    }

    canaccel=false;

    // This is the 256-byte PCI config space information for the
    // card pointed to by QWS_CARD_SLOT, as read from /proc/bus/pci
    // (or in theory from a PCI bus scan - there is some code for this
    // but it's not how Qt/Embedded would normally work)
    // It only tests the vendor ID - so don't use it with other 3dfx
    // graphics cards, such as Voodoo 4/5, or Bad Things
    // May Happen
    const unsigned char* config = qt_probe_bus();

    unsigned short int * manufacturer=(unsigned short int *)config;
    if(*manufacturer!=0x121a) {
	qDebug("This does not appear to be a 3Dfx card");
	qDebug("Are you sure QWS_CARD_SLOT is pointing to the right entry in "
	       "/proc/bus/pci?");
	return FALSE;
    }

    const unsigned char * bar=config+0x10;
    const unsigned long int * addr=(const unsigned long int *)bar;
    // We expect the address pointer for the registers in config space
    // (the 1st address specified) to be a memory one, so we do a simple 
    // sanity check
    unsigned long int s=*(addr+0); // First registers pointer
    unsigned long int olds=s;
    if(s & 0x1) {
#ifdef DEBUG_INIT
	printf("IO space - not right\n");
#endif
	return FALSE;
    } else {
#ifdef DEBUG_INIT
	qDebug("First address thing look right");
#endif
	s=s >> 1;
	s=s >> 2;
	s=olds;
	unsigned char * membase;
	int aperturefd;
	// We map in the registers from /dev/mem, which is memory
	// as seen from a physical-address point of view (rather than
	// the application's virtual address space) but including PCI-mapped
	// memory
	aperturefd=open("/dev/mem",O_RDWR);
	if(aperturefd==-1) {
#ifdef DEBUG_INIT
	    qDebug("Can't open /dev/mem");
#endif
	    return FALSE;
	}
	s=(s >> 4) << 4;
#ifdef DEBUG_INIT
	qDebug("Using physical address %lx, mapping %d",s,0x5fffff);
#endif
	membase=(unsigned char *)mmap(0,0x5fffff,PROT_READ |
				      PROT_WRITE,MAP_SHARED,
				      aperturefd,s);
	if(membase==0 || membase==(unsigned char *)-1) {
#ifdef DEBUG_INIT
	    qDebug("Failure to mmap /dev/mem, offset %d, %s",s,
		   strerror(errno));
#endif
	    close(aperturefd);
	    return FALSE;
	}
	voodoo_regbase=membase;
    }

    qDebug("Detected Voodoo 3");

    // Yes, we detected the card correctly so can safely make accelerated
    // gfxen
    canaccel=true;

    return TRUE;
}


QVoodooScreen::~QVoodooScreen()
{
}

// Set up some known values; unlike the Mach64 driver we rely more on
// the Linux framebuffer driver or Video BIOS to set up sensible default
// values; it seems to work

bool QVoodooScreen::initDevice()
{
    QLinuxFbScreen::initDevice();

    voodoo_wait_for_fifo(2);
    voodoo_regw(LINESTIPPLE,0xffffffff);
    voodoo_regw(LINESTYLE,0);

    return true;
}

void QVoodooScreen::shutdownDevice()
{
    QLinuxFbScreen::shutdownDevice();
}

int QVoodooScreen::initCursor(void* e, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    extern bool qws_sw_cursor;

    if(qws_sw_cursor==true) {
	return QLinuxFbScreen::initCursor(e,init);
    }
    qt_screencursor=new QVoodooCursor();
    qt_screencursor->init(0,false);
#else
    Q_UNUSED(e);
    Q_UNUSED(init);
#endif
    return 0;
}

QGfx * QVoodooScreen::createGfx(unsigned char * b,int w,int h,int d,
				int linestep)
{
    QGfx * ret=0;
    if( onCard(b) ) {
	if( d==16 ) {
	    ret = new QGfxVoodoo<16,0>(b,w,h);
	} else if ( d==32 ) {
	    ret = new QGfxVoodoo<32,0>(b,w,h);
	} else if ( d==8 ) {
	    ret = new QGfxVoodoo<8,0>(b,w,h);
	}
	if(ret) {
	    ret->setLineStep(linestep);
	    return ret;
	}
    }
    return QLinuxFbScreen::createGfx(b,w,h,d,linestep);
}

extern bool qws_accel;

extern "C" QScreen * qt_get_screen_voodoo3( int display_id )
{
    return new QVoodooScreen( display_id );
}

#ifndef QT_NO_QWS_CURSOR

QVoodooCursor::QVoodooCursor()
{
}

QVoodooCursor::~QVoodooCursor()
{
    hide();
}

void QVoodooCursor::init(SWCursorData *,bool)
{
    myoffset=(qt_screen->width()*qt_screen->height()*qt_screen->depth())/8;
    myoffset+=sizeof(int)*4;
    fb_start=qt_screen->base();
}

// Encode RGB values into 2-bit cursor encoding - similar to
// Mach64, but the actual values are different

int voodoo_ngval(QRgb r)
{
    if(qAlpha(r)<255) {
	return 1;        // Transparent
    } else if(qBlue(r)>240) {
        return 2;        // White
    } else {
        return 0;        // Black
    }
}

void QVoodooCursor::set(const QImage& image,int hx,int hy)
{
    cursor=&image;
    hotx=hx;
    hoty=hy;

    if(cursor->isNull()) {
        qDebug("Null cursor image!");
	abort();
        return;
    }

    // 64-bit align it
    unsigned int offset=myoffset;
    if(offset & 0xf) {
	offset=(offset+16) & (!0xf);
    }

    int loopc,loopc2;

    unsigned char * tmp;

    for(loopc=0;loopc<64;loopc++) {
	tmp=fb_start+offset+(loopc*16);
	for(loopc2=0;loopc2<8;loopc2++) {
	    *(tmp++)=0xff;
	}
	for(loopc2=0;loopc2<8;loopc2++) {
	    *(tmp++)=0x00;
	}
    }

    // Write the cursor data in the image into the weird format
    // that Voodoo3 expects for cursors, which is some truly weird
    // planar format (hence the two inner loops)
    // We assume cursors are multiples of 8 pixels wide
    for(loopc=0;loopc<cursor->height();loopc++) {
	tmp=fb_start+offset+(loopc*16);
        for(loopc2=0;loopc2<(cursor->width()/8);loopc2++) {
            unsigned int v1,v2,v3,v4,v5,v6,v7,v8;
            unsigned int pos=loopc2*8;
            v8=voodoo_ngval(cursor->pixel(pos,loopc)) & 1;
            v7=voodoo_ngval(cursor->pixel(pos+1,loopc)) & 1;
            v6=voodoo_ngval(cursor->pixel(pos+2,loopc)) & 1;
            v5=voodoo_ngval(cursor->pixel(pos+3,loopc)) & 1;
            v4=voodoo_ngval(cursor->pixel(pos+4,loopc)) & 1;
            v3=voodoo_ngval(cursor->pixel(pos+5,loopc)) & 1;
            v2=voodoo_ngval(cursor->pixel(pos+6,loopc)) & 1;
            v1=voodoo_ngval(cursor->pixel(pos+7,loopc)) & 1;
            unsigned char put=(v8 << 7) | (v7 << 6) | (v6 << 5) |
			      (v5 << 4) | (v4 << 3) | (v3 << 2) |
			      (v2 << 1) | v1;
            *(tmp++)=put;
        }
	int add=8-(cursor->width()/8);
	tmp+=add;
        for(loopc2=0;loopc2<(cursor->width()/8);loopc2++) {
            unsigned int v1,v2,v3,v4,v5,v6,v7,v8;
            unsigned int pos=loopc2*8;
            v8=voodoo_ngval(cursor->pixel(pos,loopc)) >> 1;
            v7=voodoo_ngval(cursor->pixel(pos+1,loopc)) >> 1;
            v6=voodoo_ngval(cursor->pixel(pos+2,loopc)) >> 1;
            v5=voodoo_ngval(cursor->pixel(pos+3,loopc)) >> 1;
            v4=voodoo_ngval(cursor->pixel(pos+4,loopc)) >> 1;
            v3=voodoo_ngval(cursor->pixel(pos+5,loopc)) >> 1;
            v2=voodoo_ngval(cursor->pixel(pos+6,loopc)) >> 1;
            v1=voodoo_ngval(cursor->pixel(pos+7,loopc)) >> 1;
            unsigned char put=(v8 << 7) | (v7 << 6) | (v6 << 5) |
			      (v5 << 4) | (v4 << 3) | (v3 << 2) |
			      (v2 << 1) | v1;
            *(tmp++)=put;
        }
        add=8-(cursor->width()/8);
	tmp+=add;
    }
    QRgb a=cursor->color(1);
    QRgb b=cursor->color(0);
    unsigned int c,d;
    c=(qRed(a) << 16) | (qGreen(a) << 8) | (qBlue(a) << 0);
    d=(qRed(b) << 16) | (qGreen(b) << 8) | (qBlue(b) << 0);
    voodoo_wait_for_fifo(3);
    voodoo_regw(HWCURC0,c);
    voodoo_regw(HWCURC1,d);
    voodoo_regw(HWCURPATADDR,offset);
    show();
}

// Make the accelerated cursor disappear
void QVoodooCursor::hide()
{
    unsigned int cntlstat=voodoo_regr(VIDPROCCFG);
    cntlstat=cntlstat & ~0x08000000;
    voodoo_wait_for_fifo(1);
    voodoo_regw(VIDPROCCFG,cntlstat);
}

// Make it come back
void QVoodooCursor::show()
{
    unsigned int cntlstat=voodoo_regr(VIDPROCCFG);
    cntlstat=cntlstat | 0x08000000;
    cntlstat=cntlstat & ~0x2;
    voodoo_wait_for_fifo(1);
    voodoo_regw(VIDPROCCFG,cntlstat);
}

// Move it to x,y, such that the hotspot is at x,y
void QVoodooCursor::move(int x,int y)
{
    x-=hotx;
    y-=hoty;
    x+=64;
    y+=64;
    unsigned int hold=x | (y << 16);
    voodoo_wait_for_fifo(1);
    voodoo_regw(HWCURLOC,hold);
}

#endif // QT_NO_QWS_CURSOR

#endif // QT_NO_QWS_VOODOO3
