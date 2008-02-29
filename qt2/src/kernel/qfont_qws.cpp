/****************************************************************************
** $Id: qt/src/kernel/qfont_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for FB
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

#include "qwidget.h"
#include "qpainter.h"
#include "qfontdata_p.h"
#include "qfontdatabase.h"
#include "qstrlist.h"
#include "qcache.h"
#include "qdict.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qmap.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "qfontmanager_qws.h"
#include "qmemorymanager_qws.h"

// QFont_Private accesses QFont protected functions

class QFont_Private : public QFont
{
public:
};

#undef  PRIV
#define PRIV ((QFont_Private*)this)


/*****************************************************************************
  QFontInternal contains FB font data

  Two global dictionaries and a cache hold QFontInternal objects, which
  are shared between all QFonts.
 *****************************************************************************/

class QFontInternal
{
public:
    QFontInternal( const QFontDef& );
   ~QFontInternal();
    const QFontDef *spec()  const;
    void	    reset();
    bool	    dirty() const;
    QMemoryManager::FontID handle() const
    {
	if ( !id )
	    return ((QFontInternal*)this)->id = memorymanager->refFont(s);
	return id;
    }

    int ascent() const { return memorymanager->fontAscent(handle()); }
    int descent() const { return memorymanager->fontDescent(handle()); }
    int minLeftBearing() const { return memorymanager->fontMinLeftBearing(handle()); }
    int minRightBearing() const { return memorymanager->fontMinRightBearing(handle()); }
    int leading() const { return memorymanager->fontLeading(handle()); }
    int maxWidth() const { return memorymanager->fontMaxWidth(handle()); }
    int underlinePos() const { return memorymanager->fontUnderlinePos(handle()); }
    int lineWidth() const { return memorymanager->fontLineWidth(handle()); }

private:
    QFontDef s;
    QMemoryManager::FontID id;
};

inline QFontInternal::QFontInternal( const QFontDef& d )
{
    s = d;
    id = memorymanager->refFont(d);
}

inline bool QFontInternal::dirty() const
{
    return FALSE;
}

inline const QFontDef *QFontInternal::spec() const
{
    return &s;
}

inline void QFontInternal::reset()
{
    if ( id ) {
	memorymanager->derefFont(id);
	id = 0;
    }
}

inline QFontInternal::~QFontInternal()
{
    reset();
}


static const int fontCacheSize = 64; // number of (partially) loaded fonts

class QFontCache : public QCache<QFontInternal>
{
public:
    QFontCache( int maxCost, int size=17 )
	: QCache<QFontInternal>(maxCost,size) {}
    ~QFontCache() { clear(); }
    void deleteItem( Item );
};

void QFontCache::deleteItem( Item d )
{
    QFontInternal *fin = (QFontInternal *)d;
    fin->reset();
}



typedef QDict<QFontInternal>	      QFontDict;
typedef QDictIterator<QFontInternal>  QFontDictIt;


static QFontCache    *fontCache	     = 0;	// cache of loaded fonts
static QFontDict     *fontDict	     = 0;	// dict of all loaded fonts

void qws_clearLoadedFonts()
{
    if (!fontDict)
        return;
    QFontDictIt it( *fontDict );
    while ( it.current() ) {
	QFontInternal *f = it.current();
	++it;
	f->reset();
    }
}
						// default character set:
QFont::CharSet QFont::defaultCharSet = QFont::AnyCharSet;

/*****************************************************************************
  QFont member functions
 *****************************************************************************/

/*****************************************************************************
  set_local_font() - tries to set a sensible default font char set
 *****************************************************************************/

void QFont::locale_init()
{
}

void QFont::initialize()
{
    fontCache = new QFontCache( fontCacheSize, 29 );
    fontDict  = new QFontDict( 29 );
    CHECK_PTR( fontDict );
}

void QFont::cleanup()
{
    qws_clearLoadedFonts();
    delete fontCache;
    if ( fontDict )
	fontDict->setAutoDelete( TRUE );
    delete fontDict;
}

void QFont::cacheStatistics()
{
}

// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT (d->req.dirty || d->fin->dirty())


HANDLE QFont::handle() const
{
    if ( DIRTY_FONT )
	load(); // the REAL reason this is called
    return d->fin->handle();
}

QString QFont::rawName() const
{
    if ( DIRTY_FONT )
	load();
    return "unknown";
}

void QFont::setRawName( const QString & )
{
}


bool QFont::dirty() const
{
    return DIRTY_FONT;
}


QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Times:
	    return QString::fromLatin1("times");
	case Courier:
	    return QString::fromLatin1("courier");
	case Decorative:
	    return QString::fromLatin1("old english");
	case Helvetica:
	case System:
	default:
	    return QString::fromLatin1("helvetica");
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
    qFatal( "QFont::lastResortFont: Cannot find any reasonable font" );
    // Shut compiler up
    return "Times";
}

/*
static void resetFontDef( QFontDef *def )	// used by initFontInfo()
{
    def->pointSize     = 0;
    def->styleHint     = QFont::AnyStyle;
    def->weight	       = QFont::Normal;
    def->italic	       = FALSE;
    def->charSet       = QFont::Latin1;
    def->underline     = FALSE;
    def->strikeOut     = FALSE;
    def->fixedPitch    = FALSE;
    def->hintSetByUser = FALSE;
    def->lbearing      = SHRT_MIN;
    def->rbearing      = SHRT_MIN;
}
*/

void QFont::initFontInfo() const
{
}

void QFont::load() const
{
    QString k = key();
    QFontInternal* fin = fontCache->find(k);
    if ( !fin ) {
	fin = fontDict->find(k);
	if ( !fin ) {
	    fin = new QFontInternal(d->req);
	    fontDict->insert(k,fin);
	    // make larger fonts cost a little more
	    fontCache->insert(k,fin,1+d->req.pointSize/80);
	}
    }
    d->fin = fin;
    d->req.dirty = FALSE;
}


/*****************************************************************************
  QFont_Private member functions
 *****************************************************************************/

/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/


QFontInternal *QFontMetrics::internal()
{
    if (painter) {
	painter->cfont.handle();
        return painter->cfont.d->fin;
    } else {
        return fin;
    }
}

const QFontDef *QFontMetrics::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}

// How to calculate metrics from ink and logical rectangles.
#define LBEARING(i,l) (i.x+l.x)
#define RBEARING(i,l) (i.width-l.width)
#define ASCENT(i,l) (-i.y)
#define DESCENT(i,l) (i.height+i.y-1)


int QFontMetrics::ascent() const
{
    int ret=((QFontMetrics*)this)->internal()->ascent();
    return ret;
}

int QFontMetrics::descent() const
{
    int ret=((QFontMetrics*)this)->internal()->descent();
    return ret;
}

bool QFontMetrics::inFont(QChar ch) const
{
    return memorymanager->inFont(((QFontMetrics*)this)->internal()->handle(),ch);
}

int QFontMetrics::leftBearing(QChar ch) const
{
    return memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->internal()->handle(),ch)->bearingx;
}


int QFontMetrics::rightBearing(QChar ch) const
{
    QGlyphMetrics *metrics = memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->internal()->handle(),ch);
    return metrics->advance - metrics->width - metrics->bearingx;
}

int QFontMetrics::minLeftBearing() const
{
    return ((QFontMetrics*)this)->internal()->minLeftBearing();
}

int QFontMetrics::minRightBearing() const
{
    return ((QFontMetrics*)this)->internal()->minRightBearing();
}

int QFontMetrics::height() const
{
    return ascent()+descent()+1;
}

int QFontMetrics::leading() const
{
    return ((QFontMetrics*)this)->internal()->leading();
}

int QFontMetrics::lineSpacing() const
{
    return leading() + height();
}

int QFontMetrics::width( QChar ch ) const
{
    return memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->internal()->handle(),ch)->advance;
}

int QFontMetrics::width( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();
    int ret=0;
    for (int i=0; i<len; i++)
	ret += width(str[i]);
    return ret;
}

QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    return QRect( 0,-(ascent()),width(str,len),height());
}

int QFontMetrics::maxWidth() const
{
    return ((QFontMetrics*)this)->internal()->maxWidth();
}

int QFontMetrics::underlinePos() const
{
    return ((QFontMetrics*)this)->internal()->underlinePos();
}

int QFontMetrics::strikeOutPos() const
{
    return ascent()/3; // XXX
}

int QFontMetrics::lineWidth() const
{
    return ((QFontMetrics*)this)->internal()->lineWidth();
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

const QFontDef *QFontInfo::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

const QTextCodec* QFontData::mapper() const
{
    return 0;
}

void* QFontData::fontSet() const
{
    return 0;
}


int QFont::pixelSize() const
{
    return d->req.pointSize/10;
}

void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat( pixelSize );
}

/*!
  Saves the glyphs in the font that have previously been accessed
  as a QPF file. If \a all is TRUE (the default), then before saving,
  all glyphs are marked as used.

  If the font is large and you are sure that only a subset of characters
  will ever be required on the target device, passing FALSE for the
  \a all parameter can save significant disk space.

  \note Only applicable on Qt/Embedded.
*/
void QFont::qwsRenderToDisk(bool all)
{
#ifndef QT_NO_QWS_SAVEFONTS
    memorymanager->savePrerenderedFont(handle(), all);
#else
    Q_UNUSED(all);
#endif
}
