/****************************************************************************
** $Id: qt/src/kernel/qfontmanager_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of font rendering infrastructure for Embedded Qt
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

#ifndef QFONTMANAGER_QWS_H
#define QFONTMANAGER_QWS_H

#ifndef QT_H
#include "qfont.h"
#include "qlist.h"
#endif // QT_H

// These are stored in the shared memory segment in front of their
// data, and indexed at the start of the segment

// This needs to be a multiple of 64 bits

class Q_PACKED QGlyphMetrics {

public:
    Q_UINT8 linestep;
    Q_UINT8 width;
    Q_UINT8 height;
    Q_UINT8 flags;

    Q_INT8 bearingx;      // Difference from pen position to glyph's left bbox
    Q_UINT8 advance;       // Difference between pen positions
    Q_INT8 bearingy;      // Used for putting characters on baseline

    Q_INT8 reserved;      // Do not use

    // Flags:
    // RendererOwnsData - the renderer is responsible for glyph data
    //                    memory deletion otherwise QGlyphTree must
    //                    delete [] the data when the glyph is deleted.
    enum Flags { RendererOwnsData=0x01 };
};

class QGlyph {
public:
    QGlyph() { metrics=0; data=0; }
    QGlyph(QGlyphMetrics* m, uchar* d) :
	metrics(m), data(d) { }
    ~QGlyph() {}

    QGlyphMetrics* metrics;
    uchar* data;
};



class QFontFactory;
class QDiskFont;

// This is a particular font instance at a particular resolution
// e.g. Truetype Times, 10 point. There's only one of these though;
// we want to share generated glyphs

class QRenderedFont {

public:

    // Normal font-type is monochrome; glyph data is a
    //   bitmap, which doesn't use much memory

    // Initialise for name A, renderer B, font type C, D glyphs

    QRenderedFont(QDiskFont *,const QFontDef&);
    virtual ~QRenderedFont();

    QFontDef fontDef() const;

    int refcount;

    int ptsize;

    bool italic;
    unsigned int weight;

    void ref() { refcount++; }
    bool deref() { refcount--; return refcount==0; }

    QDiskFont* diskfont;
    int fascent,fdescent;
    int fleftbearing,frightbearing;
    int fmaxwidth;
    int fleading;
    int funderlinepos;
    int funderlinewidth;
    bool smooth;
    int maxchar;

    int ascent() { return fascent; }
    int descent() { return fdescent; }
    int width(int);
    int width( const QString&, int =-1 );
    int leftBearing(int);
    int rightBearing(int);

    // Calling any of these can trigger a full-font metrics check
    // which can be expensive
    int minLeftBearing();
    int minRightBearing();
    int maxWidth();

    virtual bool inFont(QChar ch) const=0;
    virtual QGlyph render(QChar)=0;

private:

};

// Keeps track of available renderers and which font is which

class QDiskFontPrivate {};

class QDiskFont {

public:
    QDiskFont(QFontFactory *f, const QString& n, bool i, int w, int s,
	      const QString &fl, const QString& fi) :
	factory(f), name(n), italic(i), weight(w), size(s), flags(fl), file(fi)
    {
	loaded=false;
	p=0;
    }

    QRenderedFont* load(const QFontDef &);

    QFontDef fontDef() const;

    QFontFactory *factory;
    QString name;
    bool italic;
    int weight;
    int size;
    QString flags;
    QString file;
    bool loaded;

    QDiskFontPrivate * p;
};

class QFontManager {

public:

    QList<QFontFactory> factories;
    QList<QDiskFont> diskfonts;

    QFontManager();
    ~QFontManager();

    // Font definition, type and color
    QDiskFont * get(const QFontDef &);

    static int cmpFontDef(const QFontDef & goal, const QFontDef & choice);

    static void initialize();
    static void cleanup();
};

class QFontFactory {

public:

    QFontFactory() {}
    virtual ~QFontFactory() {}

    virtual QRenderedFont * get(const QFontDef &,QDiskFont *)=0;
    virtual void load(QDiskFont *) const=0;
    virtual void unload(QDiskFont *) {}
    virtual QString name()=0;
};

void qt_init_fonts();

extern QFontManager * qt_fontmanager;

#endif




