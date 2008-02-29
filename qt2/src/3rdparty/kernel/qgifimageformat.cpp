/****************************************************************************
** $Id: qt/src/3rdparty/kernel/qgifimageformat.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of gif image/movie loading classes
**
** Created : 970617
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#include "qgifimageformat_p.h"
#define Q_TRANSPARENT 0x00ffffff

/*!
  Returns TRUE if Qt was compiled with built-in GIF reading support,
  otherwise FALSE.
*/
bool qt_builtin_gif_reader()
{
#if defined(QT_BUILTIN_GIF_READER)
    return QT_BUILTIN_GIF_READER == 1;
#else
    return 0;
#endif
}

#ifndef QT_NO_ASYNC_IMAGE_IO

// See qgif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
class Q_EXPORT QGIFFormat : public QImageFormat {
public:
    QGIFFormat();
    virtual ~QGIFFormat();

    int decode(QImage& img, QImageConsumer* consumer,
	    const uchar* buffer, int length);

private:
    void fillRect(QImage&, int x, int y, int w, int h, QRgb col);
    QRgb color( uchar index ) const;

    // GIF specific stuff
    QRgb* globalcmap;
    QRgb* localcmap;
    QImage backingstore;
    unsigned char hold[16];
    bool gif89;
    int count;
    int ccount;
    int expectcount;
    enum State {
	Header,
	LogicalScreenDescriptor,
	GlobalColorMap,
	LocalColorMap,
	Introducer,
	ImageDescriptor,
	TableImageLZWSize,
	ImageDataBlockSize,
	ImageDataBlock,
	ExtensionLabel,
	GraphicControlExtension,
	ApplicationExtension,
	NetscapeExtensionBlockSize,
	NetscapeExtensionBlock,
	SkipBlockSize,
	SkipBlock,
	Done,
	Error
    } state;
    int gncols;
    int lncols;
    int ncols;
    int lzwsize;
    bool lcmap;
    int swidth, sheight;
    int left, top, right, bottom;
    enum Disposal { NoDisposal, DoNotChange, RestoreBackground, RestoreImage };
    Disposal disposal;
    bool disposed;
    int trans_index;
    bool gcmap;
    int bgcol;
    int interlace;
    int accum;
    int bitcount;

    enum { max_lzw_bits=12 }; // (poor-compiler's static const int)

    int code_size, clear_code, end_code, max_code_size, max_code;
    int firstcode, oldcode, incode;
    short table[2][1<< max_lzw_bits];
    short stack[(1<<(max_lzw_bits))*2];
    short *sp;
    bool needfirst;
    int x, y;
    int frame;
    bool out_of_bounds;
    bool digress;
    void nextY(QImage& img, QImageConsumer* consumer);
    void disposePrevious( QImage& img, QImageConsumer* consumer );
};

/* -- NOTDOC
  \class QGIFFormat qasyncimageio.h
  \brief Incremental image decoder for GIF image format.

  \ingroup images

  This subclass of QImageFormat decodes GIF format images,
  including animated GIFs.  Internally in
*/

/*!
  Constructs a QGIFFormat.
*/
QGIFFormat::QGIFFormat()
{
    globalcmap = 0;
    localcmap = 0;
    lncols = 0;
    gncols = 0;
    disposal = NoDisposal;
    out_of_bounds = FALSE;
    disposed = TRUE;
    frame = -1;
    state = Header;
    count = 0;
    lcmap = FALSE;
}

/*!
  Destructs a QGIFFormat.
*/
QGIFFormat::~QGIFFormat()
{
    if (globalcmap) delete[] globalcmap;
    if ( localcmap ) delete[] localcmap;
}


/* -- NOTDOC
  \class QGIFFormatType qasyncimageio.h
  \brief Incremental image decoder for GIF image format.

  \ingroup images

  This subclass of QImageFormatType recognizes GIF
  format images, creating a QGIFFormat when required.  An instance
  of this class is created automatically before any other factories,
  so you should have no need for such objects.
*/

QImageFormat* QGIFFormatType::decoderFor(
    const uchar* buffer, int length)
{
    if (length < 6) return 0;
    if (buffer[0]=='G'
     && buffer[1]=='I'
     && buffer[2]=='F'
     && buffer[3]=='8'
     && (buffer[4]=='9' || buffer[4]=='7')
     && buffer[5]=='a')
	return new QGIFFormat;
    return 0;
}

const char* QGIFFormatType::formatName() const
{
    return "GIF";
}


void QGIFFormat::disposePrevious( QImage& img, QImageConsumer* consumer )
{
    if ( out_of_bounds ) // flush anything that survived
	consumer->changed(QRect(0,0,swidth,sheight));

    // Handle disposal of previous image before processing next one

    if ( disposed ) return;

    int l = QMIN(swidth-1,left);
    int r = QMIN(swidth-1,right);
    int t = QMIN(sheight-1,top);
    int b = QMIN(sheight-1,bottom);

    switch (disposal) {
      case NoDisposal:
	break;
      case DoNotChange:
	break;
      case RestoreBackground:
	if (trans_index>=0) {
	    // Easy:  we use the transparent color
	    fillRect(img, l, t, r-l+1, b-t+1, Q_TRANSPARENT);
	} else if (bgcol>=0) {
	    // Easy:  we use the bgcol given
	    fillRect(img, l, t, r-l+1, b-t+1, color(bgcol));
	} else {
	    // Impossible:  We don't know of a bgcol - use pixel 0
	    QRgb** line = (QRgb **)img.jumpTable();
	    fillRect(img, l, t, r-l+1, b-t+1, line[0][0]);
	}
	if (consumer)
	    consumer->changed(QRect(l, t, r-l+1, b-t+1));
	break;
      case RestoreImage: {
	if ( frame > 0 ) {
	    QRgb** line = (QRgb **)img.jumpTable();
	    for (int ln=t; ln<=b; ln++) {
		memcpy(line[ln]+l,
		    backingstore.scanLine(ln-t),
		    (r-l+1)*sizeof(QRgb) );
	    }
	    consumer->changed(QRect(l, t, r-l+1, b-t+1));
	}
      }
    }
    disposal = NoDisposal; // Until an extension says otherwise.

    disposed = TRUE;
}

/*!
  This function decodes some data into image changes.

  Returns the number of bytes consumed.
*/
int QGIFFormat::decode(QImage& img, QImageConsumer* consumer,
	const uchar* buffer, int length)
{
    // We are required to state that
    //    "The Graphics Interchange Format(c) is the Copyright property of
    //    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    //    CompuServe Incorporated."

#define LM(l, m) (((m)<<8)|l)
    digress = FALSE;
    int initial = length;
    QRgb** line = (QRgb **)img.jumpTable();
    while (!digress && length) {
	length--;
	unsigned char ch=*buffer++;
	switch (state) {
	  case Header:
	    hold[count++]=ch;
	    if (count==6) {
		// Header
		gif89=(hold[3]!='8' || hold[4]!='7');
		state=LogicalScreenDescriptor;
		count=0;
	    }
	    break;
	  case LogicalScreenDescriptor:
	    hold[count++]=ch;
	    if (count==7) {
		// Logical Screen Descriptor
		swidth=LM(hold[0], hold[1]);
		sheight=LM(hold[2], hold[3]);
		gcmap=!!(hold[4]&0x80);
		//UNUSED: bpchan=(((hold[4]&0x70)>>3)+1);
		//UNUSED: gcmsortflag=!!(hold[4]&0x08);
		gncols=2<<(hold[4]&0x7);
		bgcol=(gcmap) ? hold[5] : -1;
		//aspect=hold[6] ? double(hold[6]+15)/64.0 : 1.0;

		trans_index = -1;
		count=0;
		ncols=gncols;
		if (gcmap) {
		    ccount=0;
		    state=GlobalColorMap;
		    globalcmap = new QRgb[gncols+1]; // +1 for trans_index
		    globalcmap[gncols] = Q_TRANSPARENT;
		} else {
		    state=Introducer;
		}
	    }
	    break;
	  case GlobalColorMap: case LocalColorMap:
	    hold[count++]=ch;
	    if (count==3) {
		QRgb rgb = qRgb(hold[0], hold[1], hold[2]);
		if ( state == LocalColorMap ) {
		    if ( ccount < lncols )
			localcmap[ccount] =  rgb;
		} else {
		    globalcmap[ccount] = rgb;
		}
		if (++ccount >= ncols) {
		    if ( state == LocalColorMap )
			state=TableImageLZWSize;
		    else
			state=Introducer;
		}
		count=0;
	    }
	    break;
	  case Introducer:
	    hold[count++]=ch;
	    switch (ch) {
	      case ',':
		state=ImageDescriptor;
		break;
	      case '!':
		state=ExtensionLabel;
		break;
	      case ';':
		if (consumer) {
		    if ( out_of_bounds ) // flush anything that survived
			consumer->changed(QRect(0,0,swidth,sheight));
		    consumer->end();
		}
		state=Done;
		break;
	      default:
		digress=TRUE;
		// Unexpected Introducer - ignore block
		state=Error;
	    }
	    break;
	  case ImageDescriptor:
	    hold[count++]=ch;
	    if (count==10) {
		int newleft=LM(hold[1], hold[2]);
		int newtop=LM(hold[3], hold[4]);
		int width=LM(hold[5], hold[6]);
		int height=LM(hold[7], hold[8]);

		// disbelieve ridiculous logical screen sizes,
		// unless the image frames are also large.
		if ( swidth/10 > QMAX(width,200) )
		    swidth = -1;
		if ( sheight/10 > QMAX(height,200) )
		    sheight = -1;

		if ( swidth <= 0 )
		    swidth = newleft + width;
		if ( sheight <= 0 )
		    sheight = newtop + height;

		if (img.isNull()) {
		    img.create(swidth, sheight, 32);
		    memset( img.bits(), 0, img.numBytes() );
		    if (consumer) consumer->setSize(swidth, sheight);
		}
		img.setAlphaBuffer(trans_index >= 0);
		line = (QRgb **)img.jumpTable();

		disposePrevious( img, consumer );
		disposed = FALSE;

		left = newleft;
		top = newtop;

		// Sanity check frame size - must fit on "screen".
		if (left >= swidth) left=QMAX(0, swidth-1);
		if (top >= sheight) top=QMAX(0, sheight-1);
		if (left+width >= swidth) {
		    if ( width <= swidth )
			left=swidth-width;
		    else
			width=swidth-left;
		}
		if (top+height >= sheight) {
		    if ( height <= sheight )
			top=sheight-height;
		    else
			height=sheight-top;
		}

		right=QMAX( 0, left+width-1);
		bottom=QMAX(0, top+height-1);
		lcmap=!!(hold[9]&0x80);
		interlace=!!(hold[9]&0x40);
		//bool lcmsortflag=!!(hold[9]&0x20);
		lncols=lcmap ? (2<<(hold[9]&0x7)) : 0;
		if (lncols) {
		    if ( localcmap )
			delete [] localcmap;
		    localcmap = new QRgb[lncols+1];
		    localcmap[lncols] = Q_TRANSPARENT;
		    ncols = lncols;
		} else {
		    ncols = gncols;
		}
		frame++;
		if ( frame == 0 ) {
		    if ( left || top || width!=swidth || height!=sheight ) {
			// Not full-size image - erase with bg or transparent
			if ( trans_index >= 0 ) {
			    fillRect(img, 0, 0, swidth, sheight, color(trans_index));
			    if (consumer) consumer->changed(QRect(0,0,swidth,sheight));
			} else if ( bgcol>=0 ) {
			    fillRect(img, 0, 0, swidth, sheight, color(bgcol));
			    if (consumer) consumer->changed(QRect(0,0,swidth,sheight));
			}
		    }
		}

		if ( disposal == RestoreImage ) {
		    int l = QMIN(swidth-1,left);
		    int r = QMIN(swidth-1,right);
		    int t = QMIN(sheight-1,top);
		    int b = QMIN(sheight-1,bottom);
		    int w = r-l+1;
		    int h = b-t+1;

		    if (backingstore.width() < w
			|| backingstore.height() < h) {
			// We just use the backing store as a byte array
			backingstore.create( QMAX(backingstore.width(), w),
					     QMAX(backingstore.height(), h),
					     32);
			memset( img.bits(), 0, img.numBytes() );
		    }
		    for (int ln=0; ln<h; ln++) {
			memcpy(backingstore.scanLine(ln),
			       line[t+ln]+l, w*sizeof(QRgb));
		    }
		}

		count=0;
		if (lcmap) {
		    ccount=0;
		    state=LocalColorMap;
		} else {
		    state=TableImageLZWSize;
		}
		x = left;
		y = top;
		accum = 0;
		bitcount = 0;
		sp = stack;
		needfirst = FALSE;
		out_of_bounds = FALSE;
	    }
	    break;
	  case TableImageLZWSize: {
	    lzwsize=ch;
	    if ( lzwsize > max_lzw_bits ) {
		state=Error;
	    } else {
		code_size=lzwsize+1;
		clear_code=1<<lzwsize;
		end_code=clear_code+1;
		max_code_size=2*clear_code;
		max_code=clear_code+2;
		int i;
		for (i=0; i<clear_code && i<(1<<max_lzw_bits); i++) {
		    table[0][i]=0;
		    table[1][i]=i;
		}
		for (i=clear_code; i<(1<<max_lzw_bits); i++) {
		    table[0][i]=table[1][i]=0;
		}
		state=ImageDataBlockSize;
	    }
	    count=0;
	    break;
	  } case ImageDataBlockSize:
	    expectcount=ch;
	    if (expectcount) {
		state=ImageDataBlock;
	    } else {
		if (consumer) {
		    consumer->frameDone();
		    digress = TRUE;
		}

		state=Introducer;
	    }
	    break;
	  case ImageDataBlock:
	    count++;
	    accum|=(ch<<bitcount);
	    bitcount+=8;
	    while (bitcount>=code_size && state==ImageDataBlock) {
		int code=accum&((1<<code_size)-1);
		bitcount-=code_size;
		accum>>=code_size;

		if (code==clear_code) {
		    if (!needfirst) {
			int i;
			code_size=lzwsize+1;
			max_code_size=2*clear_code;
			max_code=clear_code+2;
			for (i=0; i<clear_code; i++) {
			    table[0][i]=0;
			    table[1][i]=i;
			}
			for (i=clear_code; i<(1<<max_lzw_bits); i++) {
			    table[0][i]=table[1][i]=0;
			}
		    }
		    needfirst=TRUE;
		} else if (code==end_code) {
		    bitcount = -32768;
		    // Left the block end arrive
		} else {
		    if (needfirst) {
			firstcode=oldcode=code;
			if (!out_of_bounds && line && firstcode!=trans_index)
			    line[y][x] = color(firstcode);
			x++;
			if (x>=swidth) out_of_bounds = TRUE;
			needfirst=FALSE;
			if (x>right) {
			    x=left;
			    if (out_of_bounds)
				out_of_bounds = left>=swidth || y>=sheight;
			    nextY(img,consumer);
			}
		    } else {
			incode=code;
			if (code>=max_code) {
			    *sp++=firstcode;
			    code=oldcode;
			}
			while (code>=clear_code) {
			    *sp++=table[1][code];
			    if (code==table[0][code]) {
				state=Error;
				break;
			    }
			    if (sp-stack>=(1<<(max_lzw_bits))*2) {
				state=Error;
				break;
			    }
			    code=table[0][code];
			}
			*sp++=firstcode=table[1][code];
			code=max_code;
			if (code<(1<<max_lzw_bits)) {
			    table[0][code]=oldcode;
			    table[1][code]=firstcode;
			    max_code++;
			    if ((max_code>=max_code_size)
			     && (max_code_size<(1<<max_lzw_bits)))
			    {
				max_code_size*=2;
				code_size++;
			    }
			}
			oldcode=incode;
			while (sp>stack) {
			    --sp;
			    if (!out_of_bounds && *sp!=trans_index)
				line[y][x] = color(*sp);
			    x++;
			    if (x>=swidth) out_of_bounds = TRUE;
			    if (x>right) {
				x=left;
				if (out_of_bounds)
				    out_of_bounds = left>=swidth || y>=sheight;
				nextY(img,consumer);
			    }
			}
		    }
		}
	    }
	    if (count==expectcount) {
		count=0;
		state=ImageDataBlockSize;
	    }
	    break;
	  case ExtensionLabel:
	    switch (ch) {
	    case 0xf9:
		state=GraphicControlExtension;
		break;
	    case 0xff:
		state=ApplicationExtension;
		break;
#if 0
	    case 0xfe:
		state=CommentExtension;
		break;
	    case 0x01:
		break;
#endif
	    default:
		state=SkipBlockSize;
	    }
	    count=0;
	    break;
	  case ApplicationExtension:
	    if (count<11) hold[count]=ch;
	    count++;
	    if (count==hold[0]+1) {
		if (qstrncmp((char*)(hold+1), "NETSCAPE", 8)==0) {
		    // Looping extension
		    state=NetscapeExtensionBlockSize;
		} else {
		    state=SkipBlockSize;
		}
		count=0;
	    }
	    break;
	  case NetscapeExtensionBlockSize:
	    expectcount=ch;
	    count=0;
	    if (expectcount) state=NetscapeExtensionBlock;
	    else state=Introducer;
	    break;
	  case NetscapeExtensionBlock:
	    if (count<3) hold[count]=ch;
	    count++;
	    if (count==expectcount) {
		int loop = hold[0]+hold[1]*256;

		// Why if the extension here, if it is supposed to only
		// play through once?  We assume that the creator meant
		// 0, which is infinite.
		if (loop == 1) loop = 0;

		if (consumer) consumer->setLooping(loop);
		state=SkipBlockSize; // Ignore further blocks
	    }
	    break;
	  case GraphicControlExtension:
	    if (count<5) hold[count]=ch;
	    count++;
	    if (count==hold[0]+1) {
		disposePrevious( img, consumer );
		disposal=Disposal((hold[1]>>2)&0x7);
		//UNUSED: waitforuser=!!((hold[1]>>1)&0x1);
		int delay=count>3 ? LM(hold[2], hold[3]) : 1;
		// IE and mozilla use a minimum delay of 10. With the minumum delay of 10
		// we are compatible to them and avoid huge loads on the app and xserver.
		if ( delay < 10 )
		    delay = 10;

		bool havetrans=hold[1]&0x1;
 		trans_index = havetrans ? hold[4] : -1;

		if (consumer) consumer->setFramePeriod(delay*10);
		count=0;
		state=SkipBlockSize;
	    }
	    break;
	  case SkipBlockSize:
	    expectcount=ch;
	    count=0;
	    if (expectcount) state=SkipBlock;
	    else state=Introducer;
	    break;
	  case SkipBlock:
	    count++;
	    if (count==expectcount) state=SkipBlockSize;
	    break;
	  case Done:
	    digress=TRUE;
	    /* Netscape ignores the junk, so we do too.
	    length++; // Unget
	    state=Error; // More calls to this is an error
	    */
	    break;
	  case Error:
	    return -1; // Called again after done.
	}
    }
    return initial-length;
}

void QGIFFormat::fillRect(QImage& img, int col, int row, int w, int h, QRgb color)
{
    if (w>0) {
	QRgb** line = (QRgb **)img.jumpTable() + row;
	for (int j=0; j<h; j++) {
	    for ( int i=0; i<w; i++ ) {
		*(line[j]+col+i) = color;
	    }
	}
    }
}

void QGIFFormat::nextY(QImage& img, QImageConsumer* consumer)
{
    int my;
    switch (interlace) {
      case 0:
	// Non-interlaced
	if (consumer && !out_of_bounds)
	    consumer->changed(QRect(left, y, right-left+1, 1));
	y++;
	break;
      case 1:
	{
	    int i;
	    my = QMIN(7, bottom-y);
	    if ( trans_index < 0 ) // Don't dup with transparency
		for (i=1; i<=my; i++)
		    memcpy(img.scanLine(y+i)+left, img.scanLine(y)+left,
			(right-left+1)*sizeof(QRgb));
	    if (consumer && !out_of_bounds)
		consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=8;
	    if (y>bottom) {
		interlace++; y=top+4;
		if (y > bottom) { // for really broken GIFs with bottom < 5
		    interlace=2;
		    y = top + 2;
		    if (y > bottom) { // for really broken GIF with bottom < 3
			interlace = 0;
			y = top + 1;
		    }
		}
	    }
	} break;
      case 2:
	{
	    int i;
	    my = QMIN(3, bottom-y);
	    if ( trans_index < 0 ) // Don't dup with transparency
		for (i=1; i<=my; i++)
		    memcpy(img.scanLine(y+i)+left, img.scanLine(y)+left,
			(right-left+1)*sizeof(QRgb));
	    if (consumer && !out_of_bounds)
		consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=8;
	    if (y>bottom) {
		interlace++; y=top+2;
		if (y > bottom) { // for really broken GIF with bottom < 3
		    interlace = 3;
		    y = top + 1;
		}
	    }
	} break;
      case 3:
	{
	    int i;
	    my = QMIN(1, bottom-y);
	    if ( trans_index < 0 ) // Don't dup with transparency
		for (i=1; i<=my; i++)
		    memcpy(img.scanLine(y+i)+left, img.scanLine(y)+left,
			(right-left+1)*sizeof(QRgb));
	    if (consumer && !out_of_bounds)
		consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=4;
	    if (y>bottom) { interlace++; y=top+1; }
	} break;
      case 4:
	if (consumer && !out_of_bounds)
	    consumer->changed(QRect(left, y, right-left+1, 1));
	y+=2;
    }

    // Consume bogus extra lines
    if (y >= sheight) out_of_bounds=TRUE; //y=bottom;
}

QRgb QGIFFormat::color( uchar index ) const
{
    if ( index == trans_index || index > ncols )
	return Q_TRANSPARENT;
    QRgb *map = lcmap ? localcmap : globalcmap;
    return map ? map[index] : 0;
}



#endif // QT_BUILTIN_GIF_READER

#endif // QT_NO_ASYNC_IMAGE_IO
