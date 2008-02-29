/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfontengine_p.h"
#include <private/qunicodetables_p.h>
#include <qwsdisplay_qws.h>
#include <qvarlengtharray.h>
#include <private/qpainter_p.h>
#include <private/qpaintengine_raster_p.h>
#include <private/qpdf_p.h>
#include "qtextengine_p.h"
#include "qopentype_p.h"

#include <qdebug.h>


#ifndef QT_NO_QWS_QPF

#include "qfile.h"
#include "qdir.h"

#define QT_USE_MMAP
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

#endif // QT_NO_QWS_QPF




#ifndef QT_NO_FREETYPE

#include FT_TRUETYPE_TABLES_H

FT_Library QFontEngineFT::ft_library = 0;

class QGlyph {
public:
    QGlyph() :
        advance(0),
        data(0) {}
    ~QGlyph() { delete[] data; }

    quint8 pitch;
    quint8 width;
    quint8 height;

    qint8 bearingx;      // Difference from pen position to glyph's left bbox
    quint8 advance;       // Difference between pen positions
    qint8 bearingy;      // Used for putting characters on baseline

    bool mono :1;
    uint reserved:15;
    uchar* data;
};

static void render(FT_Face face, glyph_t index, QGlyph *result, bool smooth)
{
    FT_Error err;

    err=FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
    if (err) {
        qDebug("failed loading glyph %d from font", index);
        Q_ASSERT(!err);
    }

    if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        FT_Render_Mode render_mode = FT_RENDER_MODE_NORMAL;
        if (!smooth)
            render_mode = FT_RENDER_MODE_MONO;
        err = FT_Render_Glyph(face->glyph, render_mode);
        if (err && err != -1 && face->glyph->bitmap.width != 0 && face->glyph->bitmap.rows != 0) {
            qDebug("failed rendering glyph %d from font", index);
            Q_ASSERT(!err );
        }
    }

    FT_Bitmap bm = face->glyph->bitmap;
    result->pitch = bm.pitch;
    result->width = bm.width;
    result->height = bm.rows;
    result->bearingx = face->glyph->metrics.horiBearingX/64;
    result->advance = face->glyph->metrics.horiAdvance/64;
    result->bearingy = face->glyph->metrics.horiBearingY/64;
    if (result->pitch == 0 || result->height == 0 || result->width == 0) {
        result->pitch = 0;
        result->data = 0;
        return;
    }

    int size = bm.pitch*bm.rows;
    result->mono = bm.pixel_mode == ft_pixel_mode_mono;
    if (size) {
        result->data = new uchar[size];
        memcpy(result->data, bm.buffer, size);
    } else {
        result->data = 0;
    }
}


FT_Face QFontEngineFT::handle() const
{
    return face;
}

QFontEngineFT::QFontEngineFT(const QFontDef& d, FT_Face ft_face, bool antialias)
{
    _openType = 0;
    fontDef = d;
    face = ft_face;

    smooth = FT_IS_SCALABLE(face) && antialias;
    if (fontDef.styleStrategy & QFont::NoAntialias)
        smooth = false;

//    outline_drawing = xsize > (64<<6) || ysize > (64<<6);
    outline_drawing = fontDef.pixelSize > 48 && FT_IS_SCALABLE(face);

    if (outline_drawing) {
        rendered_glyphs = 0;
    } else {
    rendered_glyphs = new QGlyph *[face->num_glyphs];
    memset(rendered_glyphs, 0, face->num_glyphs*sizeof(QGlyph *));
    }
    cache_cost = face->num_glyphs*6*8; // ##########
    memset(cmapCache, 0, sizeof(cmapCache));
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (os2)
        fsType = os2->fsType;
    if (!FT_IS_SCALABLE(face)) // can't embed non scalable fonts
        fsType = 2;
}

QFontEngineFT::~QFontEngineFT()
{
    if (rendered_glyphs) {
    for (int i = 0; i < face->num_glyphs; ++i)
        delete rendered_glyphs[i];
    delete [] rendered_glyphs;
    }
    FT_Done_Face(face);
}

#endif // QT_NO_FREETYPE

inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

#ifndef QT_NO_FREETYPE

/* returns 0 as glyph index for non existant glyphs */
bool QFontEngineFT::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    int glyph_pos = 0;
    if (flags & QTextEngine::RightToLeft) {
        for ( int i = 0; i < len; ++i ) {
            unsigned int uc = QUnicodeTables::mirroredChar(getChar(str, i, len));
            glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph) {
                glyphs[glyph_pos].glyph = FT_Get_Char_Index(face, uc);
                if (uc < cmapCacheSize)
                    cmapCache[uc] = glyphs[glyph_pos].glyph;
            }
            ++glyph_pos;
        }
    } else {
        for ( int i = 0; i < len; ++i ) {
            unsigned int uc = getChar(str, i, len);
            glyphs[glyph_pos].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph) {
                glyphs[glyph_pos].glyph = FT_Get_Char_Index(face, uc);
                if (uc < cmapCacheSize)
                    cmapCache[uc] = glyphs[glyph_pos].glyph;
            }
            ++glyph_pos;
        }
    }
    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);
    glyph_pos = 0;
    return true;
}


void QFontEngineFT::draw(QPaintEngine *p, qreal _x, qreal _y, const QTextItemInt &ti)
{
    QPaintEngineState *pState = p->state;
    QRasterPaintEngine *paintEngine = static_cast<QRasterPaintEngine*>(p);

    QMatrix matrix = pState->matrix();
    matrix.translate(_x, _y);
    QFixed x = QFixed::fromReal(matrix.dx());
    QFixed y = QFixed::fromReal(matrix.dy());

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    for(int i = 0; i < glyphs.size(); i++) {
        const QGlyph *glyph = rendered_glyphs[glyphs[i]];
        Q_ASSERT(glyph);

        if(glyph->data)
            paintEngine->alphaPenBlt(glyph->data, glyph->pitch, glyph->mono,
                                     qRound(positions[i].x) + glyph->bearingx,
                                     qRound(positions[i].y) - glyph->bearingy,
                                     glyph->width,glyph->height);
    }
}

glyph_metrics_t QFontEngineFT::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    QFixed w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -ascent(), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t QFontEngineFT::boundingBox(glyph_t glyph)
{
    if (outline_drawing) {
        FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);
        FT_GlyphSlot g = face->glyph;
        return glyph_metrics_t(g->metrics.horiBearingX/64, -g->metrics.horiBearingY/64,
                               g->metrics.width/64, g->metrics.height/64, g->metrics.horiAdvance/64, 0);

    } else {
    const QGlyph *g = rendered_glyphs[glyph];
    Q_ASSERT(g);
    return glyph_metrics_t(g->bearingx, -g->bearingy,
                            g->width, g->height,
                            g->advance, 0);
    }
}


static void addGlyphToPath(FT_GlyphSlot g, const QFixedPoint &point, QPainterPath *path, bool no_scale = false)
{
    qreal factor = no_scale ? 1. : 1./64.;

    QPointF cp = point.toPointF();

    // convert the outline to a painter path
    int i = 0;
    for (int c = 0; c < g->outline.n_contours; ++c) {
        int last_point = g->outline.contours[c];
        QPointF start = cp + QPointF(g->outline.points[i].x*factor, -g->outline.points[i].y*factor);
        if(!(g->outline.tags[i] & 1)) {
            start += cp + QPointF(g->outline.points[last_point].x*factor, -g->outline.points[last_point].y*factor);
            start /= 2;
        }
//                 qDebug("contour: %d -- %d", i, g->outline.contours[c]);
//                 qDebug("first point at %f %f", start.x(), start.y());
        path->moveTo(start);

        QPointF c[4];
        c[0] = start;
        int n = 1;
        while (i < last_point) {
            ++i;
            c[n] = cp + QPointF(g->outline.points[i].x*factor, -g->outline.points[i].y*factor);
//                     qDebug() << "    i=" << i << " flag=" << (int)g->outline.tags[i] << "point=" << c[n];
            ++n;
            switch (g->outline.tags[i] & 3) {
            case 2:
                // cubic bezier element
                if (n < 4)
                    continue;
                c[3] = (c[3] + c[2])/2;
                --i;
                break;
            case 0:
                // quadratic bezier element
                if (n < 3)
                    continue;
                c[3] = (c[1] + c[2])/2;
                c[2] = (2*c[1] + c[3])/3;
                c[1] = (2*c[1] + c[0])/3;
                --i;
                break;
            case 1:
            case 3:
                if (n == 2) {
//                             qDebug() << "lineTo" << c[1];
                    path->lineTo(c[1]);
                    c[0] = c[1];
                    n = 1;
                    continue;
                } else if (n == 3) {
                    c[3] = c[2];
                    c[2] = (2*c[1] + c[3])/3;
                    c[1] = (2*c[1] + c[0])/3;
                }
                break;
            }
//                     qDebug() << "cubicTo" << c[1] << c[2] << c[3];
            path->cubicTo(c[1], c[2], c[3]);
            c[0] = c[3];
            n = 1;
        }
        if (n == 1) {
//                     qDebug() << "closeSubpath";
            path->closeSubpath();
        } else {
            c[3] = start;
            if (n == 2) {
                c[2] = (2*c[1] + c[3])/3;
                c[1] = (2*c[1] + c[0])/3;
            }
//                     qDebug() << "cubicTo" << c[1] << c[2] << c[3];
            path->cubicTo(c[1], c[2], c[3]);
        }
        ++i;
    }
}


void QFontEngineFT::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs,
                                    QPainterPath *path, QTextItem::RenderFlags)
{
    for (int gl = 0; gl < numGlyphs; gl++) {
        FT_UInt glyph = glyphs[gl];

        FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);

        FT_GlyphSlot g = face->glyph;
        if (g->format != FT_GLYPH_FORMAT_OUTLINE)
            continue;
        addGlyphToPath(g, positions[gl], path);
    }
}

void QFontEngineFT::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (FT_IS_SCALABLE(face)) {
        QFontEngine::addOutlineToPath(x, y, glyphs, numGlyphs, path, flags);
    } else {
        addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
    }
}

bool QFontEngineFT::canRender(const QChar *string,  int len)
{
    int glyph_pos = 0;
    for ( int i = 0; i < len; ++i ) {
        unsigned int uc = getChar(string, i, len);
        if (uc == 0xa0) uc = 0x20;
        if (!FT_Get_Char_Index(face, uc))
            return false;
        ++glyph_pos;
    }

    return true;
}

#define FLOOR(x)  ((x) & -64)
#define CEIL(x)   (((x)+63) & -64)
#define TRUNC(x)  ((x) >> 6)

QFixed QFontEngineFT::ascent() const
{
    return QFixed::fromFixed(face->size->metrics.ascender);
}

QFixed QFontEngineFT::descent() const
{
    return QFixed::fromFixed(-face->size->metrics.descender);
}

QFixed QFontEngineFT::leading() const
{
    return QFixed::fromFixed(face->size->metrics.height
            - face->size->metrics.ascender /*ascent*/
            + face->size->metrics.descender);
}

qreal QFontEngineFT::maxCharWidth() const
{
    return face->size->metrics.max_advance/64.;
}

QFixed QFontEngineFT::xHeight() const
{
    TT_PCLT *pct = (TT_PCLT *)FT_Get_Sfnt_Table(face, ft_sfnt_pclt);
    if (pct && pct->xHeight) {
        return QFixed(pct->xHeight*face->size->metrics.y_ppem)/face->units_per_EM;
    }
    return QFontEngine::xHeight();
}

qreal QFontEngineFT::minLeftBearing() const
{
    return 0;
//     return (memorymanager->fontMinLeftBearing(handle()))>>8;
}

qreal QFontEngineFT::minRightBearing() const
{
    return 0;
//     return (memorymanager->fontMinRightBearing(handle()))>>8;
}

QFixed QFontEngineFT::underlinePosition() const
{
    if (FT_IS_SCALABLE(face))
        return QFixed::fromFixed(-FT_MulFix(face->underline_position, face->size->metrics.y_scale));
    else
         return ((lineThickness() * 2) + 3) / 6;
}

QFixed QFontEngineFT::lineThickness() const
{
    if (FT_IS_SCALABLE(face))
        return QFixed::fromFixed(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale));

// copied from QFontEngineQPF

    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

QFontEngine::Type QFontEngineFT::type() const
{
    return Freetype;
}

QOpenType *QFontEngineFT::openType() const
{
#ifdef QT_NO_OPENTYPE
    return 0;
#else
//     qDebug("openTypeIface requested!");
    if (_openType)
        return _openType;

    if (!FT_IS_SFNT(face))
        return 0;

    QFontEngineFT *that = const_cast<QFontEngineFT *>(this);
    that->_openType = new QOpenType(that, that->face);
    return _openType;
#endif // QT_NO_OPENTYPE
}

void QFontEngineFT::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (flags & QTextEngine::DesignMetrics || outline_drawing) {
        for (int i = 0; i < len; i++) {
            int load_flags = FT_LOAD_NO_HINTING;
            if (outline_drawing)
                load_flags |= FT_LOAD_NO_BITMAP;
            FT_Load_Glyph(face, glyphs[i].glyph, load_flags);
            glyphs[i].advance.x = QFixed::fromFixed(face->glyph->metrics.horiAdvance);
            glyphs[i].advance.y = 0;
        }
    } else {
        for (int i = 0; i < len; i++) {
            FT_UInt g = glyphs[i].glyph;
            if (!rendered_glyphs[g]) {
                rendered_glyphs[g] = new QGlyph;
                render(face, g, rendered_glyphs[g], smooth);
            }
            glyphs[i].advance.x = rendered_glyphs[g]->advance;
            glyphs[i].advance.y = 0;
        }
    }
}

void QFontEngineFT::doKerning(int num_glyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    if (FT_HAS_KERNING(face)) {
        uint f = (flags == QTextEngine::DesignMetrics ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT);
        for (int i = 0; i < num_glyphs-1; ++i) {
            FT_Vector kerning;
            FT_Get_Kerning(face, glyphs[i].glyph, glyphs[i+1].glyph, f, &kerning);
            glyphs[i].advance.x += QFixed::fromFixed(kerning.x);
            glyphs[i].advance.y += QFixed::fromFixed(kerning.y);
        }
    }
}


QFontEngine::Properties QFontEngineFT::properties() const
{
    Properties p;
    QByteArray psname = FT_Get_Postscript_Name(face);
    if (psname.isEmpty()) {
        fontDef.family.toUtf8();
    }
#ifndef QT_NO_PRINTER
    p.postscriptName = QPdf::stripSpecialCharacters(psname);
#else
    p.postscriptName = psname;
#endif
#if 0 // doesn't link somehow, but who would want Type1 fonts on embedded anyway.
    PS_FontInfoRec font_info;
    if (FT_Get_PS_Font_Info(face, &font_info) == 0)
        p.copyright = font_info.notice;
#endif
    if (FT_IS_SCALABLE(face)) {
        p.ascent = face->ascender;
        p.descent = -face->descender;
        p.leading = face->height - face->ascender + face->descender;
        p.emSquare = face->units_per_EM;
        p.boundingBox = QRectF(face->bbox.xMin, -face->bbox.yMax,
                               face->bbox.xMax - face->bbox.xMin,
                               face->bbox.yMax - face->bbox.yMin);
    } else {
        p.ascent = QFixed::fromFixed(face->size->metrics.ascender);
        p.descent = QFixed::fromFixed(-face->size->metrics.descender);
        p.leading = QFixed::fromFixed(face->size->metrics.height - face->size->metrics.ascender + face->size->metrics.descender);
        p.emSquare = 1;
        p.boundingBox = QRectF(-p.ascent.toReal(), 0, (p.ascent + p.descent).toReal(), face->size->metrics.max_advance/64.);
    }
    p.italicAngle = 0;
    p.capHeight = p.ascent;
    p.lineWidth = face->underline_thickness;
    return p;
}

void QFontEngineFT::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP|FT_LOAD_NO_SCALE);

    int left  = face->glyph->metrics.horiBearingX;
    int right = face->glyph->metrics.horiBearingX + face->glyph->metrics.width;
    int top    = face->glyph->metrics.horiBearingY;
    int bottom = face->glyph->metrics.horiBearingY - face->glyph->metrics.height;

    metrics->width = right-left;
    metrics->height = top-bottom;
    metrics->x = left;
    metrics->y = -top;
    metrics->xoff = face->glyph->advance.x;
    QFixedPoint p;
    p.x = 0;
    p.y = 0;
    addGlyphToPath(face->glyph, p, path, true /* no_scale */);
}

QByteArray QFontEngineFT::getSfntTable(uint tag) const
{
    if (!FT_IS_SFNT(face))
        return QByteArray();
    FT_ULong length = 0;
    FT_Load_Sfnt_Table(face, tag, 0, 0, &length);
    QByteArray table;
    if (length != 0) {
        table.resize(length);
        FT_Load_Sfnt_Table(face, tag, 0, (FT_Byte *)table.data(), &length);
    }
    return table;
}

int QFontEngineFT::synthesized() const
{
    int s = 0;
    if ((fontDef.style != QFont::StyleNormal) && !(face->style_flags & FT_STYLE_FLAG_ITALIC))
        s = SynthesizedItalic;
    if (fontDef.stretch != 100 && FT_IS_SCALABLE(face))
        s |= SynthesizedStretch;
    return s;
}


#endif // QT_NO_FREETYPE


#ifndef QT_NO_QWS_QPF


#define FM_SMOOTH 1


class Q_PACKED QPFGlyphMetrics {

public:
    quint8 linestep;
    quint8 width;
    quint8 height;
    quint8 flags;

    qint8 bearingx;      // Difference from pen position to glyph's left bbox
    quint8 advance;       // Difference between pen positions
    qint8 bearingy;      // Used for putting characters on baseline

    qint8 reserved;      // Do not use

    // Flags:
    // RendererOwnsData - the renderer is responsible for glyph data
    //                    memory deletion otherwise QPFGlyphTree must
    //                    delete [] the data when the glyph is deleted.
    enum Flags { RendererOwnsData=0x01 };
};

class QPFGlyph {
public:
    QPFGlyph() { metrics=0; data=0; }
    QPFGlyph(QPFGlyphMetrics* m, uchar* d) :
	metrics(m), data(d) { }
    ~QPFGlyph() {}

    QPFGlyphMetrics* metrics;
    uchar* data;
};

struct Q_PACKED QPFFontMetrics{
    qint8 ascent,descent;
    qint8 leftbearing,rightbearing;
    quint8 maxwidth;
    qint8 leading;
    quint8 flags;
    quint8 underlinepos;
    quint8 underlinewidth;
    quint8 reserved3;
};


class QPFGlyphTree {
public:
    /* reads in a tree like this:

       A-Z
       /   \
       0-9   a-z

       etc.

    */
    glyph_t min,max;
    QPFGlyphTree* less;
    QPFGlyphTree* more;
    QPFGlyph* glyph;
public:
#ifdef QT_USE_MMAP
    QPFGlyphTree(uchar*& data)
    {
        read(data);
    }
#else
    QPFGlyphTree(QIODevice& f)
    {
        read(f);
    }
#endif

    ~QPFGlyphTree()
    {
        // NOTE: does not delete glyph[*].metrics or .data.
        //       the caller does this (only they know who owns
        //       the data).  See clear().
        delete less;
        delete more;
        delete [] glyph;
    }

    bool inFont(glyph_t g) const
    {
        if ( g < min ) {
            if ( !less )
                return false;
            return less->inFont(g);
        } else if ( g > max ) {
            if ( !more )
                return false;
            return more->inFont(g);
        }
        return true;
    }

    QPFGlyph* get(glyph_t g)
    {
        if ( g < min ) {
            if ( !less )
                return 0;
            return less->get(g);
        } else if ( g > max ) {
            if ( !more )
                return 0;
            return more->get(g);
        }
        return &glyph[g - min];
    }
    int totalChars() const
    {
        if ( !this ) return 0;
        return max-min+1 + less->totalChars() + more->totalChars();
    }
    int weight() const
    {
        if ( !this ) return 0;
        return 1 + less->weight() + more->weight();
    }

    void dump(int indent=0)
    {
        for (int i=0; i<indent; i++) printf(" ");
        printf("%d..%d",min,max);
        //if ( indent == 0 )
        printf(" (total %d)",totalChars());
        printf("\n");
        if ( less ) less->dump(indent+1);
        if ( more ) more->dump(indent+1);
    }

private:
    QPFGlyphTree()
    {
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
        uchar rw = *data++;
        uchar cl = *data++;
        min = (rw << 8) | cl;
        rw = *data++;
        cl = *data++;
        max = (rw << 8) | cl;
        int flags = *data++;
        if ( flags & 1 )
            less = new QPFGlyphTree;
        else
            less = 0;
        if ( flags & 2 )
            more = new QPFGlyphTree;
        else
            more = 0;
        int n = max-min+1;
        glyph = new QPFGlyph[n];

        if ( less )
            less->readNode(data);
        if ( more )
            more->readNode(data);
    }
#else
    void readNode(QIODevice& f)
    {
        uchar rw = f.getch();
        uchar cl = f.getch();
        min = (rw << 8) | cl;
        rw = f.getch();
        cl = f.getch();
        max = (rw << 8) | cl;
        int flags = f.getch();
        if ( flags & 1 )
            less = new QPFGlyphTree;
        else
            less = 0;
        if ( flags & 2 )
            more = new QPFGlyphTree;
        else
            more = 0;
        int n = max-min+1;
        glyph = new QPFGlyph[n];

        if ( less )
            less->readNode(f);
        if ( more )
            more->readNode(f);
    }
#endif

#ifdef QT_USE_MMAP
    void readMetrics(uchar*& data)
    {
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            glyph[i].metrics = (QPFGlyphMetrics*)data;
            data += sizeof(QPFGlyphMetrics);
        }
        if ( less )
            less->readMetrics(data);
        if ( more )
            more->readMetrics(data);
    }
#else
    void readMetrics(QIODevice& f)
    {
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            glyph[i].metrics = new QPFGlyphMetrics;
            f.readBlock((char*)glyph[i].metrics, sizeof(QPFGlyphMetrics));
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
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
            //######### s = qt_screen->mapToDevice( s );
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
        int n = max-min+1;
        for (int i=0; i<n; i++) {
            QSize s( glyph[i].metrics->width, glyph[i].metrics->height );
            //############### s = qt_screen->mapToDevice( s );
            uint datasize = glyph[i].metrics->linestep * s.height();
            glyph[i].data = new uchar[datasize]; // ### deleted?
            f.readBlock((char*)glyph[i].data, datasize);
        }
        if ( less )
            less->readData(f);
        if ( more )
            more->readData(f);
    }
#endif

};

class QFontEngineQPFData
{
public:
    QPFFontMetrics fm;
    QPFGlyphTree *tree;
};


QFontEngineQPF::QFontEngineQPF(const QFontDef&, const QString &fn)
{
    cache_cost = 1;

    int f = ::open( QFile::encodeName(fn), O_RDONLY );
    Q_ASSERT(f>=0);
    struct stat st;
    if ( fstat( f, &st ) )
        qFatal("Failed to stat %s",QFile::encodeName(fn).data());
    uchar* data = (uchar*)mmap( 0, // any address
                                st.st_size, // whole file
                                PROT_READ, // read-only memory
#if !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX4)
                                MAP_FILE | MAP_PRIVATE, // swap-backed map from file
#else
                                MAP_PRIVATE,
#endif
                                f, 0 ); // from offset 0 of f
#if defined(Q_OS_QNX4) && !defined(MAP_FAILED)
#define MAP_FAILED ((void *)-1)
#endif
    if ( !data || data == (uchar*)MAP_FAILED )
        qFatal("Failed to mmap %s",QFile::encodeName(fn).data());
    ::close(f);

    d = new QFontEngineQPFData;
    memcpy(reinterpret_cast<char*>(&d->fm),data,sizeof(d->fm));

    data += sizeof(d->fm);
    d->tree = new QPFGlyphTree(data);

#if 0
    qDebug() << "font file" << fn
             << "ascent" << d->fm.ascent << "descent" << d->fm.descent
             << "leftbearing" << d->fm.leftbearing
             << "rightbearing" << d->fm.rightbearing
             << "maxwidth" << d->fm.maxwidth
             << "leading" << d->fm.leading
             << "flags" << d->fm.flags
             << "underlinepos" << d->fm.underlinepos
             << "underlinewidth" << d->fm.underlinewidth;
#endif
}

QFontEngineQPF::~QFontEngineQPF()
{
    delete d->tree;
    delete d;
}


bool QFontEngineQPF::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags) const
{
    if(*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    *nglyphs = 0;

    for(int i = 0; i < len; i++) {
        unsigned int uc = getChar(str, i, len);
        glyphs[*nglyphs].glyph = uc < 0x10000 ? uc : 0;
        ++*nglyphs;
    }

    for(int i = 0; i < len; i++) {
        QGlyphLayout &g=glyphs[i];
        QPFGlyph *glyph = d->tree->get(g.glyph);

        g.advance.x = glyph ? glyph->metrics->advance : 0;
        g.advance.y = 0;
    }

    return true;
}

void QFontEngineQPF::draw(QPaintEngine *p, qreal _x, qreal _y, const QTextItemInt &si)
{
    QPaintEngineState *pState = p->state;
    QRasterPaintEngine *paintEngine = static_cast<QRasterPaintEngine*>(p);

    QMatrix matrix = pState->matrix();
    matrix.translate(_x, _y);
    QFixed x = QFixed::fromReal(matrix.dx());
    QFixed y = QFixed::fromReal(matrix.dy());

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    getGlyphPositions(si.glyphs, si.num_glyphs, matrix, si.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    for(int i = 0; i < glyphs.size(); i++) {
        const QPFGlyph *glyph = d->tree->get(glyphs[i]);
        if (!glyph)
            continue;


        int mono = !(d->fm.flags & FM_SMOOTH);
        int bpl = glyph->metrics->linestep;

        if(glyph->data)
            paintEngine->alphaPenBlt(glyph->data, bpl, mono,
                                     qRound(positions[i].x) + glyph->metrics->bearingx,
                                     qRound(positions[i].y) - glyph->metrics->bearingy,
                                     glyph->metrics->width,glyph->metrics->height);
    }
}


void QFontEngineQPF::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

glyph_metrics_t QFontEngineQPF::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
   if (numGlyphs == 0)
        return glyph_metrics_t();

    QFixed w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -ascent(), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t QFontEngineQPF::boundingBox(glyph_t glyph)
{
    const QPFGlyph *g = d->tree->get(glyph);
    if (!g)
        return glyph_metrics_t();
    Q_ASSERT(g);
    return glyph_metrics_t(g->metrics->bearingx, -g->metrics->bearingy,
                            g->metrics->width, g->metrics->height,
                            g->metrics->advance, 0);
}

QFixed QFontEngineQPF::ascent() const
{
    return d->fm.ascent;
}

QFixed QFontEngineQPF::descent() const
{
    return d->fm.descent;
}

QFixed QFontEngineQPF::leading() const
{
    return d->fm.leading;
}

qreal QFontEngineQPF::maxCharWidth() const
{
    return d->fm.maxwidth;
}
/*
const char *QFontEngineQPF::name() const
{
    return "qt";
}
*/
bool QFontEngineQPF::canRender(const QChar *str, int len)
{
    for(int i = 0; i < len; i++)
        if (!d->tree->inFont(str[i].unicode()))
            return false;
    return true;
}

QFontEngine::Type QFontEngineQPF::type() const
{
    return QPF;
}

qreal QFontEngineQPF::minLeftBearing() const
{
    return d->fm.leftbearing;
}

qreal QFontEngineQPF::minRightBearing() const
{
    return d->fm.rightbearing;
}

QFixed QFontEngineQPF::underlinePosition() const
{
    return d->fm.underlinepos;
}

QFixed QFontEngineQPF::lineThickness() const
{
    return d->fm.underlinewidth;
}


#endif //QT_NO_QWS_QPF
