/****************************************************************************
** $Id: qt/src/kernel/qmemorymanager_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/Embedded memory management routines
**
** Created : 000101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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
#include "qmemorymanager_qws.h"
#include "qfontmanager_qws.h"
#include "qgfx_qws.h"
#include "qpaintdevice.h"
#include "qfontdata_p.h"
#include "qfile.h"
#include "qdir.h"

/*!
  \class QMemoryManager qmemorymanager_qws.h

  \brief The QMemoryManager class provides the allocation and tracking
	of memory used by pixmaps and font glyphs.

  \ingroup environment

  The QMemoryManager class allocates pixmaps from video ram, main
  memory, or now if enabled, shared memory which allows pixmap data
  to be shared between Qt/Embedded processes. The shared memory is
  managed by the QSharedMemoryManager class which is able to track
  allocated and freed memory from it between processes. It works
  similar to malloc and free in how it maintains that memory, however
  it is implemented in a way to allow this to work from different
  processes which may have this memory mapped to different virtual
  addresses as well as needing locking during allocation and deallocation.

  To make shared memory useful between processes, there needs to be
  a common way for different processes to identify and reference the share
  items stored in the memory. Therefore items stored are looked up based
  on a key, the key also needs to be in the shared memory, and if present
  can then be used to map an item in the memory based on the key, much in
  the way the pixmap cache already works but in a shared manner that
  works across processes. The key searching is implemented using an array
  with bsearch for efficency. When the items are added, qsort is used to
  keep the item pointers ordered by key so the bsearch works. The choice to
  use an array and bsearch was one of efficency, however this means that
  the number of items able to be added to the cache is limited by the size
  chosen for the array at compile time, however a future improvement could
  be to use a dynamic array allocated from shared memory to make it possible
  to grow this array as needed. But in most cases it should be possible to
  determine the average size of items in the shared memory, which means a
  reasonable value for the size of this array should be able to be determined.
  A reasonable guess maybe that items will be 1Kb in size on average, some
  debug to profile the size of items as they are added to the cache should
  make it possible to be more exact for a given environment.

  The font glyph caching is very much the same as pixmap caching if
  it is possible to generate a key that identifies the glyph or set of glyphs.
  Storage of the key is an overhead in font glyph caching, therefore this
  cost is reduced if multiple glyphs are cached at a time with the
  one key. A value of 16 glyphs at a time was chosen based on testing
  of memory usage and the trade off of key overhead caching different
  number of glyphs at a time. Originally a row of 256 glyphs were
  cached at a time, but trial and error showed that in a test of
  text with Japanese text, that typically the characters used spanned
  too many of the rows that it very quickly filled the cache and that
  the majority of the glyphs cached did not get used. The glyphs used
  tended to be from all over the range of unicode characters, and
  16 glyphs at a time was reasonable with out filling the test cache
  of 1 Mb. Caching multiple glyphs requires that an array in the cached
  item indexes the offset to the glyph in the set within the chunk of
  shared memory.

  When the cache gets full, both the font and pixmap caching
  code falls back to their original implementations of allocating and
  searching from local memory to that process. Another factor taken
  in to account is screen rotation, the key associated with an item
  in the shared memory has appended something that identifys the
  rotation of the process which added it. This ensures that after
  the rotation is changed or an app with a different rotation is
  run, it will not find from the cache an incorrectly rotated image
  than what it requires.

  As with pixmap caching previously, an item
  in the cache will remain there while there are still references
  to the data. This requires reference counting so that it is
  possible to determine when it is safe to reuse the data used by
  a cached item. To avoid delays during allocation when memory is
  required from the cache, a timer event periodically removes a few
  items at a time from the cache which have been identified as
  no longer referenced. This provides something similar to
  incremental garbage collection. It is possible that after a
  dereference and an an item is identified as needing to be removed
  it may be referenced again before the timer event, therefore the
  timer event must check again the item still has no references.

  For extra robustness, some extra locking maybe required in a few
  places. Another consideration that requires some additions to the
  current code is the abnormal termination of a process which contains
  references to items in the cache. To handle this signal handlers will be
  required to ensure items get correctly dereferenced in these cases.
  This is similar to the same problem with pixmaps allocated from
  video ram by a process which ends abnormally.

*/

#if !defined(_OS_QNX6_) // always for now
#define QT_USE_MMAP
#endif

#include <stdlib.h>

#ifdef QT_USE_MMAP
// for mmap
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include "qgfx_qws.h"

extern QString qws_topdir();

#define FM_SMOOTH 1

static void getProcessedGlyph(QGlyph& tmp, QGlyph& g, bool smooth)
{
    if ( tmp.metrics->width && tmp.metrics->height &&
	 (qt_screen->isTransformed() ) ) {
	int depth = 1;
	int cols = 0;
	QImage::Endian end = QImage::BigEndian;
	if ( smooth ) {
	    depth = 8;
	    cols = 256;
	    end = QImage::IgnoreEndian;
	}
	QImage img( tmp.data, tmp.metrics->width, tmp.metrics->height,
		    depth, tmp.metrics->linestep, 0, cols, end );
	img = qt_screen->mapToDevice( img );
	g.metrics = tmp.metrics;
	g.data = new uchar [img.numBytes()];

	memcpy( g.data, img.bits(), img.numBytes() );
	g.metrics->linestep = img.bytesPerLine();
	delete [] tmp.data;
    } else {

#ifndef QT_NO_QWS_INTERLACE
	if ( tmp.metrics->width && tmp.metrics->height &&
	     qt_screen->isInterlaced() ) {
	    int height = tmp.metrics->height + 1;
	    int width = tmp.metrics->width;
	    int pitch = tmp.metrics->linestep;
	    if ( !smooth ) {
		pitch=(width+3) & -4;
	    }
	    uchar *newdata = new uchar[height*pitch];
	    uchar *olddata = tmp.data;
	    memset(newdata,0,height*pitch);

	    if (smooth) {
		for(int y=0; y<height; y++) {
		    unsigned char *src1 = y == height-1 ? 0 :olddata+y*pitch;
		    unsigned char *src2 = y == 0 ? 0 :olddata+(y-1)*pitch;
		    unsigned char *dst = newdata+y*pitch;

		    for(int x=0;x<width;x++) {
			*dst++ = (2*(src1?*src1++:0) + (src2?*src2++:0))/3;
		    }
		}
	    } else {
		QRgb colTable[2] = {0, 1};
		QImage img( tmp.data, width, height,
			    1, tmp.metrics->linestep,
			    colTable, 2, QImage::BigEndian);
		for(int y=0; y<height; y++) {
		    unsigned char *dst = newdata+y*pitch;

		    for(int x=0;x<width;x++) {
			*dst++ = ( y!=height-1 && img.pixel(x,y)? 170 :0)
			 + ( y!=0 && img.pixel(x,y-1)? 85 :0);
		    }
		}
	
	
	    }
	
	    delete tmp.data;
	    tmp.data = newdata;
	    tmp.metrics->height = height;
	    tmp.metrics->linestep = pitch;
	}	
#endif //QT_NO_QWS_INTERLACE	
	g = tmp;
    }
}

class QGlyphTree {
    /* Builds up a tree like this:

       root
       /
       Q
       / \
       /   \
       G     l
       / \
       /   \
       /     \
       T       y
       \     /
       e   p
       / \
       /   \
       h     r

       etc.

       Which can be compressed into contiguous spans (when fuller):

       A-Z
       /   \
       0-9   a-z

       etc.

       such a compressed tree could then be stored in ROM.
    */
    QChar min,max;
    QGlyphTree* less;
    QGlyphTree* more;
    QGlyph* glyph;
public:
#ifdef QT_USE_MMAP
    QGlyphTree(uchar*& data)
    {
	read(data);
    }
#else
    QGlyphTree(QIODevice& f)
    {
	read(f);
    }
#endif

    QGlyphTree(const QChar& from, const QChar& to, QRenderedFont* renderer) :
	min(from),
	max(to),
	less(0),
	more(0)
    {
	int n = max.unicode()-min.unicode()+1;
	glyph = new QGlyph[n];
	for (int i=0; i<n; i++) {
	    QChar ch(min.unicode()+i);
	    QGlyph tmp = renderer->render(ch);
	    getProcessedGlyph(tmp,glyph[i],renderer->smooth);
	}
    }

    ~QGlyphTree()
    {
	// NOTE: does not delete glyph[*].metrics or .data.
	//       the caller does this (only they know who owns
	//	 the data).  See clear().
	delete less;
	delete more;
	delete [] glyph;
    }

    void clear()
    {
	if ( less ) less->clear();
	if ( more ) more->clear();
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    if ( glyph[i].data && !(glyph[i].metrics->flags & QGlyphMetrics::RendererOwnsData) ) {
		delete [] glyph[i].data;
	    }
	    delete glyph[i].metrics;
	}
    }

    bool inFont(const QChar& ch) const
    {
	if ( ch < min ) {
	    if ( !less )
		return FALSE;
	    return less->inFont(ch);
	} else if ( ch > max ) {
	    if ( !more )
		return FALSE;
	    return more->inFont(ch);
	}
	return TRUE;
    }

    QGlyph* get(const QChar& ch, QRenderedFont* renderer)
    {
	if ( ch < min ) {
	    if ( !less ) {
		if ( !renderer || !renderer->inFont(ch) )
		    return 0;
		less = new QGlyphTree(ch,ch,renderer);
	    }
	    return less->get(ch,renderer);
	} else if ( ch > max ) {
	    if ( !more ) {
		if ( !renderer || !renderer->inFont(ch) )
		    return 0;
		more = new QGlyphTree(ch,ch,renderer);
	    }
	    return more->get(ch,renderer);
	}
	return &glyph[ch.unicode()-min.unicode()];
    }
    int totalChars() const
    {
	if ( !this ) return 0;
	return max.unicode()-min.unicode()+1 + less->totalChars() + more->totalChars();
    }
    int weight() const
    {
	if ( !this ) return 0;
	return 1 + less->weight() + more->weight();
    }
    static void balance(QGlyphTree*& root)
    {
	int x,y;
	(void)balance(root,x,y);
    }
    static int balance(QGlyphTree*& root, int& l, int& m)
    {
	if ( root ) {
	    int ll, lm, ml, mm;
	    l = balance(root->less,ll,lm);
	    m = balance(root->more,ml,mm);

	    if ( root->more ) {
		if ( l + ml + 1 < mm ) {
		    // Shift less-ward
		    QGlyphTree* b = root;
		    QGlyphTree* c = root->more;
		    root = c;
		    b->more = c->less;
		    c->less = b;
		}
	    }
	    if ( root->less ) {
		if ( m + lm + 1 < ll ) {
		    // Shift more-ward
		    QGlyphTree* c = root;
		    QGlyphTree* b = root->less;
		    root = b;
		    c->less = b->more;
		    b->more = c;
		}
	    }
	    return 1 + l + m;
	} else {
	    l = m = 0;
	    return 0;
	}
    }
    void compress();

    void write(QIODevice& f)
    {
	writeNode(f);
	writeMetrics(f);
	writeData(f);
    }


    void dump(int indent=0)
    {
	for (int i=0; i<indent; i++) printf(" ");
	printf("%d..%d",min.unicode(),max.unicode());
	//if ( indent == 0 )
	printf(" (total %d)",totalChars());
	printf("\n");
	if ( less ) less->dump(indent+1);
	if ( more ) more->dump(indent+1);
    }

private:
    QGlyphTree()
    {
    }

    void writeNode(QIODevice& f)
    {
	f.writeBlock((char*)&min, sizeof(QChar));
	f.writeBlock((char*)&max, sizeof(QChar));
	int flags = 0;
	if ( less ) flags |= 1;
	if ( more ) flags |= 2;
	f.putch(flags);
	if ( less ) less->writeNode(f);
	if ( more ) more->writeNode(f);
    }

    void writeMetrics(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++)
	    f.writeBlock((char*)glyph[i].metrics, sizeof(QGlyphMetrics));
	if ( less ) less->writeMetrics(f);
	if ( more ) more->writeMetrics(f);
    }

    void writeData(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint datasize = glyph[i].metrics->linestep * s.height();
	    f.writeBlock((char*)glyph[i].data, datasize);
	}
	if ( less ) less->writeData(f);
	if ( more ) more->writeData(f);
    }

#ifdef QT_USE_MMAP
    void read(uchar*& data)
    {
	// All node data first
	readNode(data);
	// Then all non-video data
	readMetrics(data);
	// Then all video data
	readData(data);
    }
#else
    void read(QIODevice& f)
    {
	// All node data first
	readNode(f);
	// Then all non-video data
	readMetrics(f);
	// Then all video data
	readData(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readNode(uchar*& data)
    {
	memcpy((uchar*)&min,data,sizeof(QChar)); data += sizeof(QChar);
	memcpy((uchar*)&max,data,sizeof(QChar)); data += sizeof(QChar);
	int flags = *data++;
	if ( flags & 1 )
	    less = new QGlyphTree;
	else
	    less = 0;
	if ( flags & 2 )
	    more = new QGlyphTree;
	else
	    more = 0;
	int n = max.unicode()-min.unicode()+1;
	if ( n <= 0 )
	{
	    qWarning("corrupt data detected in QGlyphTree::readNode");
	    return;
	}
	glyph = new QGlyph[n];

	if ( less )
	    less->readNode(data);
	if ( more )
	    more->readNode(data);
    }
#else
    void readNode(QIODevice& f)
    {
	f.readBlock((char*)&min, sizeof(QChar));
	f.readBlock((char*)&max, sizeof(QChar));
	int flags = f.getch();
	if ( flags & 1 )
	    less = new QGlyphTree;
	else
	    less = 0;
	if ( flags & 2 )
	    more = new QGlyphTree;
	else
	    more = 0;
	int n = max.unicode()-min.unicode()+1;
	if ( n <= 0 )
	{
	    qWarning("corrupt data detected in QGlyphTree::readNode");
	    return;
	}
	glyph = new QGlyph[n];

	if ( less )
	    less->readNode(f);
	if ( more )
	    more->readNode(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readMetrics(uchar*& data)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    glyph[i].metrics = (QGlyphMetrics*)data;
	    data += sizeof(QGlyphMetrics);
	}
	if ( less )
	    less->readMetrics(data);
	if ( more )
	    more->readMetrics(data);
    }
#else
    void readMetrics(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    glyph[i].metrics = new QGlyphMetrics;
	    f.readBlock((char*)glyph[i].metrics, sizeof(QGlyphMetrics));
	}
	if ( less )
	    less->readMetrics(f);
	if ( more )
	    more->readMetrics(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readData(uchar*& data)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint datasize = glyph[i].metrics->linestep * s.height();
	    glyph[i].data = data; data += datasize;
	}
	if ( less )
	    less->readData(data);
	if ( more )
	    more->readData(data);
    }
#else
    void readData(QIODevice& f)
    {
	int n = max.unicode()-min.unicode()+1;
	for (int i=0; i<n; i++) {
	    QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint datasize = glyph[i].metrics->linestep * s.height();
	    glyph[i].data = new uchar[datasize];
	    f.readBlock((char*)glyph[i].data, datasize);
	}
	if ( less )
	    less->readData(f);
	if ( more )
	    more->readData(f);
    }
#endif

    static QGlyph* concatGlyphs(QGlyphTree* a, QGlyphTree* b, QChar& min, QChar& max)
    {
	int n = b->max.unicode()-a->min.unicode()+1;
	int n_a = a->max.unicode()-a->min.unicode()+1;
	QGlyph *newglyph = new QGlyph[n];
	int i=0;
	for (; i<n_a; i++)
	    newglyph[i] = a->glyph[i];
	for (; i<n; i++)
	    newglyph[i] = b->glyph[i-n_a];
	min = a->min;
	max = b->max;
	return newglyph;
    }
};

void QGlyphTree::compress()
{
    // XXX Does not compress as much as is possible

    if ( less ) {
	less->compress();
	if (less->max.unicode() == min.unicode()-1) {
	    // contiguous with me.
	    QGlyph *newglyph = concatGlyphs(less,this,min,max);
	    QGlyphTree* t = less->less; less->less = 0;
	    delete less; less = t;
	    delete [] glyph;
	    glyph = newglyph;
	}
    }

    if ( more ) {
	more->compress();
	if (more->min.unicode() == max.unicode()+1) {
	    // contiguous with me.
	    QGlyph *newglyph = concatGlyphs(this,more,min,max);
	    QGlyphTree* t = more->more; more->more = 0;
	    delete more; more = t;
	    delete [] glyph;
	    glyph = newglyph;
	}
    }
}

#define CACHE_ROW_SIZE	    16
#define CACHE_ROWS	    (65536/CACHE_ROW_SIZE)

#ifdef DEBUG_SHARED_MEMORY_CACHE
# define CHECK_ROW_CACHE_CONSISTENCY() \
    ASSERT( checkRowCacheConsistency() );
#else
# define CHECK_ROW_CACHE_CONSISTENCY()
#endif

class QMemoryManagerFont : public QShared {
    QGlyph* default_glyph;
    QGlyph tmpGlyph;
public:
    QMemoryManagerFont(QFontDef font) : default_glyph(0), def(font), renderer(0), tree(0)
    {
#if !defined(QT_NO_SHARED_FONT_CACHE)
	rowCache = 0;
#endif
	r0Cache = new QGlyph* [256];
	memset((void*)r0Cache, 0, sizeof(QGlyph*)*256);
    }
    ~QMemoryManagerFont()
    {
	if ( default_glyph ) {
	    delete default_glyph->metrics;
	    delete [] default_glyph->data;
	    delete default_glyph;
	}
	if ( tree ) {
	    if ( renderer ) {
		tree->clear();
		delete renderer;
	    }
	    delete tree;
	}
#if !defined(QT_NO_SHARED_FONT_CACHE)
	if ( rowCache ) {
	    CHECK_ROW_CACHE_CONSISTENCY();

	    bool derefGlyphRows = false;

	    if ( !sharedRowCache ) {
		derefGlyphRows = true;
	    } else {
		QSMCacheItemPtr item((char*)rowCache);
		if ( item.count() == 1 ) {
		    derefGlyphRows = true;
		    qDebug("need to deref glyph rows for %s font", fontKey());
		}
	    }
	    if ( derefGlyphRows ) {
		for ( int i = 0; i < CACHE_ROWS; i++ ) {
		    if ( (char*)rowCache[i] ) {
			QSMCacheItemPtr item((char*)rowCache[i]);
			item.deref();
			if ( sharedRowCache )
			    rowCache[i] = QSMemPtr();
		    }
		}
	    }
	    if ( sharedRowCache ) {
		qDebug("doing deref for row cache for %s font", fontKey());
		QSMCacheItemPtr item((char*)rowCache);
		item.deref();
	    } else {
		delete [] rowCache;
	    }
	}
#endif
	delete [] r0Cache;
#ifdef QT_USE_MMAP
        munmap(start_address, size);
#endif
    }

#if !defined(QT_NO_SHARED_FONT_CACHE)
# ifdef DEBUG_SHARED_MEMORY_CACHE
    bool checkRowCacheConsistency()
    {
	bool good = true;
	if ( rowCache ) {
	    for (int i = 0; i < CACHE_ROWS; i++) {
		if ( (char*)rowCache[i] ) {
		    if ( (int)rowCache[i] < -1 ) {
			qDebug("weird index at %i  (%i)", i, (int)rowCache[i]);
			good = false;
		    } else {
			QSMCacheItemPtr item((char*)rowCache[i]);
			if ( item.count() < 1 ) {
			    qDebug("not refed at %i", i);
			    good = false;
			}
		    }
		}
	    }
	}
	return good;
    }
# endif
    void findRowCache()
    {
	CHECK_ROW_CACHE_CONSISTENCY();
	if ( !rowCache ) {
	    sharedRowCache = true;
	    rowCache = (QSMemPtr*)(char*)QSharedMemoryManager::findItem(fontKey(), true, QSMCacheItem::GlyphRowIndex);
	    if ( !rowCache ) {
		rowCache = (QSMemPtr*)(char*)QSharedMemoryManager::newItem(fontKey(), CACHE_ROWS * sizeof(QSMemPtr), QSMCacheItem::GlyphRowIndex);
		if ( !rowCache ) {
		    sharedRowCache = false;
		    //qDebug("creating row cache from heap");
		    rowCache = new QSMemPtr[CACHE_ROWS];
		} else {
		    //qDebug("created row cache in shm");
		    for (int i = 0; i < CACHE_ROWS; i++)
			rowCache[i] = QSMemPtr();
		}
	    }
	}
	CHECK_ROW_CACHE_CONSISTENCY();
    }
#endif

    QGlyph* defaultGlyph()
    {
	if ( !default_glyph ) {
	    QGlyphMetrics* m = new QGlyphMetrics;
	    memset((char*)m, 0, sizeof(QGlyphMetrics));
	    m->width = fm.maxwidth;
	    m->linestep = (fm.flags & FM_SMOOTH) ? m->width : (m->width+7)/8;
	    m->height = fm.ascent;
	    m->bearingy = fm.ascent;
	    m->advance = m->width+1+m->width/8;
	    uchar* d = new uchar[m->linestep*m->height];
	    memset(d,255,m->linestep);
	    memset(d+m->linestep*(m->height-1),255,m->linestep);
	    for (int i=1; i<m->height-1; i++) {
		int j=m->linestep*i;
		memset(d+j,0,m->linestep);
		if ( fm.flags & FM_SMOOTH ) {
		    d[j]=255;
		    d[j+m->linestep-1]=255;
		} else {
		    d[j]=128;
		    d[j+m->linestep-1]|=128>>((m->width+7)%8);
		}
	    }
	    QGlyph g(m,d);
	    default_glyph = new QGlyph;
	    getProcessedGlyph(g,*default_glyph,fm.flags & FM_SMOOTH);
	}
	return default_glyph;
    }
#if !defined(QT_NO_SHARED_FONT_CACHE)
#define ROW_KEY_HEADER_SIZE	4 // 4 hex digits
    const char *fontKey()
    {
	return rowKey() + ROW_KEY_HEADER_SIZE + 1;
    }
    const char *rowKey(int row = -1)
    {
	if ( keyStrUtf8.isNull() ) {
	    QString keyStr;
	    if ( def.rawMode )
		keyStr = def.family.lower();
	    else {
		Q_UINT8 bits = ( def.italic ? 0x01 : 0 ) | ( def.underline ? 0x02 : 0 ) |
		    ( def.strikeOut ? 0x04 : 0 ) | ( def.fixedPitch ? 0x08 : 0 ) |
		    ( def.hintSetByUser ? 0x10 : 0 ) | ( def.rawMode ? 0x20 : 0 );
		keyStr.sprintf("%s%s%02i%02i%02i%02i%02i", def.family.lower().latin1(),
			def.addStyle.lower().latin1(), bits,
			def.weight, def.pointSize, def.charSet,
			def.hintSetByUser ? (int)def.styleHint : (int)QFont::AnyStyle );
	    }
	    QString filler;
	    filler.fill('_', ROW_KEY_HEADER_SIZE + 2);
	    keyStr = filler + QString("%1_$R%2").arg( keyStr ).arg( qt_screen->transformOrientation() );
#ifndef QT_NO_TEXTCODEC
            keyStrUtf8 = keyStr.utf8();
#else
	    // no text codes are enabled use the local 8 bit form of key 
            keyStrUtf8 = keyStr.local8Bit();
#endif
	}
	if ( row != -1 ) {
	    snprintf(keyStrUtf8.data(), ROW_KEY_HEADER_SIZE+2, "_%04x", row);
	    keyStrUtf8.data()[ROW_KEY_HEADER_SIZE+1] = '_';
	}
	return keyStrUtf8.data();
    }
    QGlyph *findInCache(ushort index)
    {
	CHECK_ROW_CACHE_CONSISTENCY();

	uint row = index / CACHE_ROW_SIZE;
	uint cell = index % CACHE_ROW_SIZE;

	if ( !rowCache )
	    findRowCache();
	char *rowData = (char*)rowCache[row];

	if ( !rowData ) {
            rowCache[row] = QSMemPtr((char*)QSharedMemoryManager::findItem(rowKey(row), true, QSMCacheItem::GlyphRow));
	    rowData = (char*)rowCache[row];
//	    qDebug("connected to row cache[%i] of %s", row, rowKey(row) );
	}

	if ( rowData ) {
	    ushort *cellPtrs = (ushort*)rowData;
	    ushort index = cellPtrs[cell];
	    char *ptr = &rowData[index];

	    QGlyph *g = &tmpGlyph;
	    g->metrics = (QGlyphMetrics*)ptr;
	    ptr += sizeof(QGlyphMetrics);
	    g->data = (uchar *)(((int)ptr+3)&~0x3);
	    CHECK_ROW_CACHE_CONSISTENCY();
	    return g;
	}

	CHECK_ROW_CACHE_CONSISTENCY();
	return 0;
    }
    QGlyph *addToCache(ushort index, QRenderedFont *renderer)
    {
	CHECK_ROW_CACHE_CONSISTENCY();

	ushort row = index / CACHE_ROW_SIZE;

	int reqSize = CACHE_ROW_SIZE*2;
	QGlyph *gArray[CACHE_ROW_SIZE];
	bool delFlagArray[CACHE_ROW_SIZE];

	index = row * CACHE_ROW_SIZE;
	for (int i = 0; i < CACHE_ROW_SIZE; i++, index++) {
	    QChar ch(index);
	    QGlyph *g = 0;
	    delFlagArray[i] = false;

	    if ( renderer ) {
		bool inFont = renderer->inFont(ch);
		if ( !inFont && ch.isSpace() ) {
		    ch = QChar(' ');
		    inFont = renderer->inFont(ch);
		}
		if ( inFont ) {
		    g = new QGlyph;
		    QGlyph tmp = renderer->render(ch);
// ######### optimizable, lots of moving and allocating memory about etc
		    getProcessedGlyph(tmp,*g,renderer->smooth);
		    delFlagArray[i] = true;
		}
	    }

	    if ( !g )
		g = defaultGlyph();

	    gArray[i] = g;
	    reqSize += sizeof(QGlyphMetrics);
	    reqSize = ((int)reqSize+3)&~0x3;
	    QSize s( g->metrics->width, g->metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint dataSize = g->metrics->linestep * s.height();
	    reqSize += dataSize;
	    reqSize = ((int)reqSize+3)&~0x3;
	}

	CHECK_ROW_CACHE_CONSISTENCY();
	char *ptr = (char*)QSharedMemoryManager::newItem(rowKey(row), reqSize, QSMCacheItem::GlyphRow);
	CHECK_ROW_CACHE_CONSISTENCY();

//        char *ptr = (char*)qt_getSMManager()->alloc(reqSize);
//      void free(QSMemPtr ptr);

	if ( !ptr ) {
	    for (int i = 0; i < CACHE_ROW_SIZE; i++) {
		QGlyph *g = gArray[i];
		if ( delFlagArray[i] ) {
		    delete g->metrics;
		    delete [] g->data;
		    delete g;
		}
	    }
	    CHECK_ROW_CACHE_CONSISTENCY();
	    return 0;
	}

	if ( !rowCache )
	    findRowCache();
	rowCache[row] = QSMemPtr(ptr);
	ushort *cellPtrs = (ushort*)ptr;
	ptr += CACHE_ROW_SIZE*2;

	for (int i = 0; i < CACHE_ROW_SIZE; i++) {
	    cellPtrs[i] = (ushort)(ptr - (char*)cellPtrs);
	    QGlyph *g = gArray[i];
	    memcpy(ptr,g->metrics,sizeof(QGlyphMetrics));
	    ptr += sizeof(QGlyphMetrics);
	    ptr = (char*)(((int)ptr+3)&~0x3);
	    QSize s( g->metrics->width, g->metrics->height );
	    s = qt_screen->mapToDevice( s );
	    uint dataSize = g->metrics->linestep * s.height();
	    memcpy(ptr,g->data,dataSize);
	    ptr += dataSize;
	    ptr = (char*)(((int)ptr+3)&~0x3);
	    if ( delFlagArray[i] ) {
		delete g->metrics;
		delete [] g->data;
		delete g;
	    }
	}

	char *rowData = (char*)rowCache[row];
	ptr = &rowData[cellPtrs[index % CACHE_ROW_SIZE]];
	QGlyph *g = &tmpGlyph;
	g->metrics = (QGlyphMetrics*)ptr;
	ptr += sizeof(QGlyphMetrics);
	g->data = (uchar *)(((int)ptr+3)&~0x3);
	CHECK_ROW_CACHE_CONSISTENCY();
	return g;
    }
#endif
    QGlyph *get(const QChar &ch)
    {
	QGlyph *g;
#if !defined(QT_NO_SHARED_FONT_CACHE)
	if ( renderer ) {
	    ushort index = ch.unicode();
	    g = findInCache(index);
	    if ( !g )
		g = addToCache(index, renderer);
	    if ( g )
		return g;
	}
#endif
	if (ch.row() == 0) { // optimisation for latin1
	    int cell = ch.cell();
	    g = r0Cache[cell];
	    if (!g) {
		g = r0Cache[cell] = tree->get(ch, renderer);
		if (!g) {
		    if ( ch.isSpace() && ch != ' ' )
			g = r0Cache[cell] = get(' ');
		    else
			g = r0Cache[cell] = defaultGlyph();
		}
	    }
	} else {
	    g = tree->get(ch, renderer);
	    if (!g) {
		if ( ch.isSpace() && ch != ' ' )
		    g = get(' ');
		else
		    g = defaultGlyph();
	    }
	}
	return g;
    }

    QCString keyStrUtf8;
    QFontDef def;
    QRenderedFont* renderer; // ==0 for QPFs
    QGlyphTree* tree;
#if !defined(QT_NO_SHARED_FONT_CACHE)
    QSMemPtr *rowCache;
    bool sharedRowCache;
#endif
#ifdef QT_USE_MMAP
    dev_t size;
    char* start_address;
#endif
    QGlyph **r0Cache;

    struct Q_PACKED {
	Q_INT8 ascent,descent;
	Q_INT8 leftbearing,rightbearing;
	Q_UINT8 maxwidth;
	Q_INT8 leading;
	Q_UINT8 flags;
	Q_UINT8 underlinepos;
	Q_UINT8 underlinewidth;
	Q_UINT8 reserved3;
    } fm;
};


QMemoryManager::QMemoryManager() :
    next_pixmap_id(1000)
{
}


static const int memAlign = 32;


int QMemoryManager::pixmapLinestep(PixmapID id, int width, int depth)
{
    int align = id&1 ? qt_screen->pixmapLinestepAlignment() : memAlign;
    return ((width*depth+align-1)/align)*align/8;
}


// Pixmaps
QMemoryManager::PixmapID QMemoryManager::newPixmap(int w, int h, int d,
						   int optim )
{
    // Find space in vram.
    int size = pixmapLinestep(1,w,d)*h;
    PixmapType type = VRAM;
    uchar *data = qt_screen->cache(size, optim);

#if 0 // !defined(QT_NO_SHARED_PIXMAP_CACHE)
    if ( shared ) {
        size = pixmapLinestep(1,w,d)*h;
        type = Shared;
        data = (char*)shm->cache.newItem(k.latin1(),size+sizeof(PixmapShmItem))+sizeof(PixmapShmItem),QSMCacheItem::GlyphRow);
    }
#endif

    // No vram left - use main memory
    if ( !data ) { 
        size = pixmapLinestep(0,w,d)*h;
        type = Normal;
        data = (uchar*)malloc(size);
    }

    // Out of memory
    if ( !data )
        return 0;

    memset((void *)data,0,size);
    return mapPixmapData(data, type); 
}


void QMemoryManager::deletePixmap(PixmapID id)
{
    if ( !id ) return; // C++-life
    QMap<PixmapID,uchar*>::Iterator it = pixmap_map.find(id);
    switch ( pixmapType(id) ) {
        case Normal:
	    free(*it);
            break;
        case VRAM:
	    qt_screen->uncache(*it);
            break;
#if !defined(QT_NO_SHARED_PIXMAP_CACHE)
        case Shared:
	    derefSharedMemoryPixmap(*it);
            break;
#endif
        default:
            qWarning("Invalid type of pixmap");
            break;
    }
    pixmap_map.remove(it);
}


// Fonts

static QString fontKey(const QFontDef& font)
{
    QString key = font.family.lower();

    key += "_";
    key += QString::number(font.pointSize);
    key += "_";
    key += QString::number(font.weight);
    key += font.italic ? "i" : "";
    if ( qt_screen->isTransformed() ) {
	key += "_t";
	QPoint a = qt_screen->mapToDevice(QPoint(0,0),QSize(2,2));
	QPoint b = qt_screen->mapToDevice(QPoint(1,1),QSize(2,2));
	key += QString::number( a.x()*8+a.y()*4+(1-b.x())*2+(1-b.y()) );
    }
    if ( qt_screen->isInterlaced() ) {
	key += "_I";
    }
    return key;
}

static QFontDef fontFilenameDef( const QString& key )
{
    // font name has the form of
    // (/w)+_(\d)+_(\d)+(i|) the rest of it we don't care about.
    QFontDef result;
    int u0 = key.find('_');
    int u1 = key.find('_',u0+1);

    // next field will be deleminted by either a _, . or i.
    // find the first non-diget
    int u2 = u1+1;
    while(key[u2].isDigit())
	u2++;

    result.family = key.left(u0);
    result.pointSize = key.mid(u0+1,u1-u0-1).toInt();
    result.weight = key.mid(u1+1,u2-u1-1).toInt();
    result.italic = key[u2] == 'i';
    // #### ignores _t and _I fields

    return result;
}

static QString fontDir()
{
    return qws_topdir()+"/lib/fonts/";
}

static QString fontFilename(const QFontDef& font)
{
    return fontDir()+fontKey(font)+".qpf";
}

QMemoryManager::FontID QMemoryManager::refFont(const QFontDef& font)
{
    QString key = fontKey(font);

    // look it up...

    QMap<QString,FontID>::ConstIterator i = font_map.find(key);
    QMemoryManagerFont* mmf;
    if ( i == font_map.end() ) {
	mmf = new QMemoryManagerFont(font);
	QString filename = fontFilename(font);
	if ( !QFile::exists(filename) ) {
	    filename = QString::null;
            int bestmatch = -999;
	    QDiskFont* qdf = qt_fontmanager->get(font);
#ifndef QT_NO_DIR
	    // QPF close-font matching
	    if ( qdf )
	        bestmatch = QFontManager::cmpFontDef(font, qdf->fontDef());
	    QDir dir(fontDir(),"*.qpf");
	    for (int i=0; i<(int)dir.count(); i++) {
	        QFontDef d = fontFilenameDef(dir[i]);
	        int match = QFontManager::cmpFontDef(font,d);
	        if ( match >= bestmatch ) {
		    QString ff = fontFilename(d);
		    if ( QFile::exists(ff) ) {
		        filename = ff;
		        bestmatch = match;
		    }
	        }
	    }
#endif
	    if ( filename.isNull() ) {
	        if ( qdf ) {
		    mmf->renderer = qdf->load(font);
	        } else {
		    // last-ditch attempt to find a font...
		    QFontDef d = font;
		    d.family = "helvetica";
		    filename = fontFilename(d);
		    if ( !QFile::exists(filename) ) {
		        d.pointSize = 120;
		        filename = fontFilename(d);
		        if ( !QFile::exists(filename) ) {
		            d.italic = FALSE;
		            filename = fontFilename(d);
		            if ( !QFile::exists(filename) ) {
		                d.weight = 50;
		                filename = fontFilename(d);
		            }
		        }
		    }
	        }
	    }
	}
	if ( !mmf->renderer && QFile::exists(filename) ) {
#if defined(QT_USE_MMAP)
	    int f = ::open( QFile::encodeName(filename), O_RDONLY );
	    ASSERT(f>=0);
	    struct stat st;
	    if ( fstat( f, &st ) )
		qFatal("Failed to stat %s",QFile::encodeName(filename).data());
	    uchar* data = (uchar*)mmap( 0, // any address
		    st.st_size, // whole file
                    PROT_READ, // read-only memory
#if !defined(_OS_SOLARIS_)
                    MAP_FILE | MAP_PRIVATE, // swap-backed map from file
#else
		    MAP_PRIVATE,
#endif
                    f, 0 ); // from offset 0 of f
	    if ( !data || data == (uchar*)MAP_FAILED )
		qFatal("Failed to mmap %s",QFile::encodeName(filename).data());
	    ::close(f);
            mmf->size = st.st_size;
            mmf->start_address = (char *)data;
	    memcpy((char*)&mmf->fm,data,sizeof(mmf->fm)); data += sizeof(mmf->fm);
	    mmf->tree = new QGlyphTree(data);
#else
	    QFile f(filename);
	    f.open(IO_ReadOnly);
	    f.readBlock((char*)&mmf->fm,sizeof(mmf->fm));
	    mmf->tree = new QGlyphTree(f);
#endif
	} else {
	    memset((char*)&mmf->fm, 0, sizeof(mmf->fm));
	    if(!mmf->renderer) {
		return (FontID)mmf;
	    }
	    mmf->fm.ascent = mmf->renderer->fascent;
	    mmf->fm.descent = mmf->renderer->fdescent;
	    mmf->fm.leftbearing = mmf->renderer->fleftbearing;
	    mmf->fm.rightbearing = mmf->renderer->frightbearing;
	    mmf->fm.maxwidth = mmf->renderer->fmaxwidth;
	    mmf->fm.leading = mmf->renderer->fleading;
	    mmf->fm.underlinepos = mmf->renderer->funderlinepos;
	    mmf->fm.underlinewidth = mmf->renderer->funderlinewidth;
	    mmf->fm.flags = (mmf->renderer->smooth||qt_screen->isInterlaced())
			    ? FM_SMOOTH : 0;
	}
	font_map[key] = (FontID)mmf;
#ifndef QT_NO_QWS_SAVEFONTS
	extern bool qws_savefonts;
	if ( qws_savefonts && mmf->renderer )
	    savePrerenderedFont(font);
#endif
    } else {
	mmf = (QMemoryManagerFont*)*i;
	mmf->ref();
    }
    return (FontID)mmf;
}

void QMemoryManager::derefFont(FontID id)
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    if( font->renderer ) {
        font->renderer->deref();
    }
    if ( font->deref() ) {
	QString key = fontKey(font->def);
	font_map.remove(key);
	delete font;
    }
}

QRenderedFont* QMemoryManager::fontRenderer(FontID id)
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    return font->renderer;
}

bool QMemoryManager::inFont(FontID id, const QChar& ch) const
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    if ( font->renderer )
	return ch.unicode() <= font->renderer->maxchar && font->renderer->inFont(ch);
    else
	return font->tree->inFont(ch);
}

QGlyph QMemoryManager::lockGlyph(FontID id, const QChar& ch)
{
    QMemoryManagerFont* font = (QMemoryManagerFont*)id;
    if ( !font->tree ) {
	if(!font->renderer) {
	    if ( ch.isSpace() && ch != ' ' )
		return lockGlyph(id,' ');
	    return *(font->defaultGlyph());
	}
	QChar c = ch;
	if ( !font->renderer->inFont(c) )
	    c = ' '; // ### Hope this is inFont()
	font->tree = new QGlyphTree(c,c,font->renderer);
    }
    QGlyph *g = font->get(ch);
    return *g;
}

QGlyphMetrics* QMemoryManager::lockGlyphMetrics(FontID id, const QChar& ch)
{
    QGlyph g = lockGlyph(id,ch);
    return g.metrics;
}

void QMemoryManager::unlockGlyph(FontID, const QChar&)
{
}

#ifndef QT_NO_QWS_SAVEFONTS
void QMemoryManager::savePrerenderedFont(const QFontDef& f, bool all)
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)refFont(f);
    savePrerenderedFont((FontID)mmf,all);
}

void QMemoryManager::savePrerenderedFont(FontID id, bool all)
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;

    if ( !mmf->renderer ) {
	qWarning("Already a ROM font");
    } else {
	if ( !mmf->tree )
	    mmf->tree = new QGlyphTree(32,32,mmf->renderer); // 32 = " " - likely to be in the font
	if ( all ) {
	    int j=0;
	    //qDebug("Rendering %s",fontFilename(mmf->def).ascii());
	    for (int i=0; i<=mmf->renderer->maxchar; i++) {
		QChar ch((ushort)i);
		if ( mmf->renderer->inFont(ch) ) {
		    mmf->tree->get(ch,mmf->renderer);
		    if ( !(j++ & 0x3f)  ) {
			// XXX keep it from becoming degenerate - should be in QGlyphTree
			mmf->tree->compress();
			QGlyphTree::balance(mmf->tree);
		    }
		}
	    }
	}
	mmf->tree->compress();
	QGlyphTree::balance(mmf->tree);
	//qDebug("DUMP..."); mmf->tree->dump();
	QFile f(fontFilename(mmf->def));
	f.open(IO_WriteOnly);
	f.writeBlock((char*)&mmf->fm,sizeof(mmf->fm));
	mmf->tree->write(f);
    }
}
#endif
// Perhaps we should just return the struct?
//
bool QMemoryManager::fontSmooth(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.flags & FM_SMOOTH;
}
int QMemoryManager::fontAscent(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.ascent;
}
int QMemoryManager::fontDescent(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.descent;
}
int QMemoryManager::fontMinLeftBearing(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.leftbearing;
}
int QMemoryManager::fontMinRightBearing(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.rightbearing;
}
int QMemoryManager::fontLeading(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.leading;
}
int QMemoryManager::fontMaxWidth(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.maxwidth;
}
int QMemoryManager::fontUnderlinePos(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.underlinepos ? mmf->fm.underlinepos : 1;
}
int QMemoryManager::fontLineWidth(FontID id) const
{
    QMemoryManagerFont* mmf = (QMemoryManagerFont*)id;
    return mmf->fm.underlinewidth ? mmf->fm.underlinewidth : 1;
}


QMemoryManager* memorymanager=0;
