/****************************************************************************
** $Id: qt/src/kernel/qgfxmatrox_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QGfxMatrox (graphics context) class for Matrox MGA cards
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

#include "qgfxmatroxdefs_qws.h"

#ifndef QT_NO_QWS_MATROX

#include "qgfxlinuxfb_qws.h"

#include "qimage.h"

// This is the least featureful of the accelerated drivers - use the
// Voodoo3 or Mach64 drivers as better examples. The main interesting
// feature of this driver is the rather primitive way Matrox cards
// (at least early ones) handle providing the offset and linestep of
// data. Since this seems to differ from card to card in ways I've not
// yet deciphered, on-card pixmaps are not supported

static unsigned char *matrox_regbase=0;

//#define DEBUG_INIT

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

inline unsigned int matrox_regr(volatile unsigned int regindex)
{
    unsigned long int val;
    val=*((volatile unsigned long *)(matrox_regbase+regindex));
    return val;
}

inline void matrox_regw(volatile unsigned int regindex,unsigned long val)
{
    *((volatile unsigned long int *)(matrox_regbase+regindex))=val;
}

// Older Matrox hardware has no hardware cursor; leave it for now

template <const int depth, const int type>
class QGfxMatrox : public QGfxRaster<depth,type> {

public:

    QGfxMatrox(unsigned char *,int w,int h);

    virtual void fillRect(int,int,int,int);
    virtual void blt(int,int,int,int,int,int);
    virtual void sync();

private:

    // Convert colour into what the hardware needs
    unsigned int get_color(unsigned int);

    bool checkSourceDest();
    bool checkDest();
    void do_scissors(QRect &);

    unsigned int src_pixel_offset;

};

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::do_scissors(QRect & r)
{
    matrox_regw(CXLEFT,r.left());
    matrox_regw(CXRIGHT,r.right());
    int t=this->linestep();
    t=(t*8)/depth;
    matrox_regw(YTOP,r.top()*t);
    matrox_regw(YBOT,r.bottom()*t);
}

template<const int depth,const int type>
inline unsigned int QGfxMatrox<depth,type>::get_color(unsigned int i)
{
    unsigned int ret;
    if(depth==32) {
	ret=i;
    } else if(depth==16) {
	i=i & 0xffff;
	ret=(i << 16) | i;
    } else {
	// 8
	i=i & 0xff;
	ret=(i << 24) | (i << 16) | (i << 8) | i;
    }
    return ret;
}

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::sync()
{
    int loopc;
    for(loopc=0;loopc<1000000;loopc++) {
        unsigned int stat=matrox_regr(STATUS);
	if((!(stat & 0x10000)) && loopc>20) {
	    return;
	}
    }
    qDebug("Idle timeout!");
}

template<const int depth,const int type>
inline bool QGfxMatrox<depth,type>::checkDest()
{
    ulong buffer_offset;
    if (!qt_screen->onCard(this->buffer,buffer_offset)) {
	return FALSE;
    }

    int d;
    if(depth==16) {
	d=0x1;
    } else if(depth==32) {
	d=0x2;
    } else {
	// 8bpp
	d=0;
    }

    int t=this->linestep();
    t=(t*8)/depth;
    if(t & 0x1f) {
	qDebug("Unaligned pixel linestep %d",t);
	return false;
    }
    unsigned int b=(buffer_offset >> 6) << 6;
    b=(b*8)/depth;
    matrox_regw(PITCH,0x8000 | t);
    matrox_regw(YDSTORG,b);
    matrox_regw(MACCESS,d);
    matrox_regw(CXLEFT,0);
    matrox_regw(CXRIGHT,this->width);
    matrox_regw(YTOP,(0*t)+b);
    matrox_regw(YBOT,(this->height*t)+b);
    matrox_regw(PLNWT,0xffffffff);

    return TRUE;
}

template<const int depth,const int type>
inline bool QGfxMatrox<depth,type>::checkSourceDest()
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

	src_pixel_offset=(src_buffer_offset * 8)/depth;

    }
    return TRUE;
}

template<const int depth,const int type>
QGfxMatrox<depth,type>::QGfxMatrox(unsigned char * a,int b,int c)
    : QGfxRaster<depth,type>(a,b,c)
{
}

template<const int depth,const int type>
void QGfxMatrox<depth,type>::fillRect(int rx,int ry,int w,int h)
{
    if(this->ncliprect<1) {
	return;
    }

    if( (this->cbrush.style()!=this->NoBrush) && (this->cbrush.style()!=this->SolidPattern) ) {
	QGfxRaster<depth,type>::fillRect(rx,ry,w,h);
	return;
    }

    QWSDisplay::grab( TRUE );
    if(!checkDest()) {
	QWSDisplay::ungrab();
	QGfxRaster<depth,type>::fillRect(rx,ry,w,h);
	return;
    }

    GFX_START(QRect(rx+this->xoffs, ry+this->yoffs, w+1, h+1))

    (*optype)=1;
    (*lastop)=LASTOP_RECT;

    int loopc;

    int xp=rx+this->xoffs;
    int yp=ry+this->yoffs;

    int x2,y2;

    x2=xp+(w-1);
    y2=yp+(h-1);

    int x3,y3,x4,y4;

    QColor tmp=this->cbrush.color();

    checkDest();

    matrox_regw(FCOL,get_color(tmp.alloc()));

    (*optype)=1;

    // Last in 1d00-1dff range

    if(this->cbrush.style()!=this->NoBrush) {
	int p=this->ncliprect;
	for(loopc=0;loopc<p;loopc++) {
	    QRect r=this->cliprect[loopc];
	    if(xp<=r.right() && yp<=r.bottom() &&
	       x2>=r.left() && y2>=r.top()) {
		x3 = QMAX( r.left(), xp );
		y3 = QMAX( r.top(),  yp );
		x4 = QMIN( r.right(),  x2 );
		y4 = QMIN( r.bottom(), y2 );
		int hh=(y4-y3)+1;
		int p=y3;
		int t=this->linestep();
		t=(t*8)/depth;
		//t&=0x1f;
		p*=(t >> 5);
		matrox_regw(DWGCTL,DWG_MODE);
		matrox_regw(FXLEFT,x3);
		matrox_regw(FXRIGHT,x4+1);
		matrox_regw(YDST,p);
		matrox_regw(LEN | EXEC,hh);
	    }
	}
    }
    GFX_END
    QWSDisplay::ungrab();
}

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::blt(int rx,int ry,int w,int h,int sx,int sy)
{
    if(this->ncliprect<1)
	return;

    if((depth!=16 && depth!=32 && depth!=8) || w*h/this->ncliprect < 1000 ) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
	return;
    }

    bool canaccel=false;

    if((this->srcdepth==32 || this->srcdepth==16 || this->srcdepth==8) &&
       this->alphatype==QGfx::IgnoreAlpha) {
	canaccel=true;
    }

    if(this->srctype==QGfx::SourceImage && canaccel==false) {
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
	return;
    }

    if(this->srctype==QGfx::SourcePen) {
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

	int mx = QMIN(xp,xp2);
	if ( mx < 0 ) {
	    //Matrox does not like blt to/from negative X coords
	    //so we clip against left edge of screen.
	    xp -= mx;
	    xp2 -= mx;
	    w += mx;
	}
	
	QRect cursRect(xp, yp, w+1, h+1);

	GFX_START(cursRect)

	    
        bool rev = (yp > yp2) || ((yp == yp2) && (xp > xp2));
	int dy = (yp > yp2) ? -1 : 1;
	int dx = (xp > xp2) ? -1 : 1;
	
	int loopc = (dy<0) ? this->ncliprect-1 : 0;
	
	while ( loopc >=0 && loopc < this->ncliprect ) {
	
	    int ylevel = this->cliprect[loopc].y();
	    if ( dx != dy ) {
		// find other end of strip
		while ( loopc >=0 && loopc < this->ncliprect 
			&& this->cliprect[loopc].y() == ylevel )
		    loopc -= dx;
		loopc += dx;
	    }
	    int end_of_strip = loopc;
	    do {

		do_scissors(this->cliprect[loopc]);

		int tw=w;
		int th=h;
		int start,end;
		int typ=yp;

		int src_pixel_linestep=(this->srclinestep*8)/this->srcdepth;

		if ( !rev ) {
		    matrox_regw(AR5,src_pixel_linestep);
		    matrox_regw(DWGCTL,DWG_BITBLT | DWG_SHIFTZERO | DWG_SGNZERO |
				DWG_BFCOL | DWG_REPLACE);
		    tw--;
		    start=yp2*src_pixel_linestep+xp2+src_pixel_offset;
		    end=start+tw;
		} else {
		    matrox_regw(SGN,5);
		    matrox_regw(AR5,-src_pixel_linestep);
		    matrox_regw(DWGCTL,DWG_BITBLT | DWG_SHIFTZERO | DWG_BFCOL |
				DWG_REPLACE);
		    tw--;
		    end=(yp2+th-1)*src_pixel_linestep+xp2+src_pixel_offset;
		    start=end+tw;
		    typ+=th-1;
		}
		matrox_regw(AR0,end);
		matrox_regw(AR3,start);
		matrox_regw(FXBNDRY,((xp+tw)<<16) | xp);
		int p=typ;
		int t=this->linestep();
		t=(t*8)/depth;
		//t&=0x1f;
		p*=(t >> 5);
		matrox_regw(YDST,p);
		matrox_regw(LEN | EXEC,th);

		loopc += dx;
	    } while ( loopc >= 0 && loopc < this->ncliprect && 
		      this->cliprect[loopc].y() == ylevel );
	    
	    //find next strip
	    if ( dx != dy )
		loopc = end_of_strip - dx;
	    
	}
	QRect r(0,0,this->width,this->height);
	do_scissors(r);
	QWSDisplay::ungrab();

	GFX_END

	    return;
    } else {
	QWSDisplay::ungrab();
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
    }
}


class QMatroxScreen : public QLinuxFbScreen {

public:

    QMatroxScreen( int display_id );
    virtual ~QMatroxScreen();
    virtual bool connect( const QString &spec );
    virtual bool initDevice();
    virtual void shutdownDevice();
    virtual bool useOffscreen() { return false; }

    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

protected:

    virtual int pixmapOffsetAlignment() { return 256; }
    virtual int pixmapLinestepAlignment() { return 256; }

private:

    unsigned int src_pixel_offset;

};

QMatroxScreen::QMatroxScreen( int display_id  )
    : QLinuxFbScreen( display_id )
{
}

bool QMatroxScreen::connect( const QString &spec )
{
    if (!QLinuxFbScreen::connect( spec )) {
	qDebug("Matrox driver couldn't connect to framebuffer");
	return FALSE;
    }

    canaccel=false;

    const unsigned char* config = qt_probe_bus();

    unsigned short int * manufacturer=(unsigned short int *)config;
    if(*manufacturer!=0x102b) {
	qDebug("This does not appear to be a Matrox card");
	qDebug("Are you sure QWS_CARD_SLOT is pointing to the right entry in "
	       "/proc/bus/pci?");
	return FALSE;
    }

    const unsigned char * bar=config+0x14;
    const unsigned long int * addr=(const unsigned long int *)bar;
    unsigned long int s=*(addr+0);
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
	aperturefd=open("/dev/mem",O_RDWR);
	if(aperturefd==-1) {
#ifdef DEBUG_INIT
	    qDebug("Can't open /dev/mem");
#endif
	    return FALSE;
	}
	s=(s >> 4) << 4;
#ifdef DEBUG_INIT
	qDebug("Using physical address %lx, mapping %d",s,0x3fff);
#endif
	membase=(unsigned char *)mmap(0,0x3fff,PROT_READ |
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
	matrox_regbase=membase;
    }
#ifdef DEBUG_INIT
    qDebug("Detected Matrox card");
#endif
    canaccel=true;

    return TRUE;
}


QMatroxScreen::~QMatroxScreen()
{
}

bool QMatroxScreen::initDevice()
{
    QLinuxFbScreen::initDevice();
    return true;
}

void QMatroxScreen::shutdownDevice()
{
    QLinuxFbScreen::shutdownDevice();
}

QGfx * QMatroxScreen::createGfx(unsigned char * b,int w,int h,int d,
				int linestep)
{
    QGfx * ret=0;
    if( onCard(b) ) {
	if( d==16 ) {
	    ret = new QGfxMatrox<16,0>(b,w,h);
	} else if ( d==32 ) {
	    ret = new QGfxMatrox<32,0>(b,w,h);
	} else if ( d==8 ) {
	    ret = new QGfxMatrox<8,0>(b,w,h);
	}
	if(ret) {
	    ret->setLineStep(linestep);
	    return ret;
	}
    }
    return QLinuxFbScreen::createGfx(b,w,h,d,linestep);
}

extern bool qws_accel;

extern "C" QScreen * qt_get_screen_matrox( int display_id )
{
    return new QMatroxScreen( display_id );
}

#endif // QT_NO_QWS_MATROX

