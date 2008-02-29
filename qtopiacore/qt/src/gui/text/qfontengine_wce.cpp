/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfontengine_p.h"
#include <qglobal.h>
#include "qt_windows.h"
#include "qapplication_p.h"

#include <qpaintdevice.h>
#include <qpainter.h>
#include <limits.h>
#include <math.h>

#include <private/qunicodetables_p.h>
#include <qbitmap.h>

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

HDC   shared_dc            = 0;                // common dc for all fonts


// general font engine ----------------------------------------------

QFontEngine::~QFontEngine()
{
    if (hdc) {                                // one DC per font (Win NT)
        //SelectObject(hdc, systemFont());
        if (!stockFont)
            DeleteObject(hfont);
        if (!paintDevice)
            ReleaseDC(0, hdc);
        hdc = 0;
        hfont = 0;
    }
}

// ##### get these from windows
int QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if (lw < 2 && score >= 1050) lw = 2;
    if (lw == 0) lw = 1;

    return lw;
}

// ##### get these from windows
int QFontEngine::underlinePosition() const
{
    int pos = ((lineThickness() * 2) + 3) / 6;
    return pos ? pos : 1;
}


HDC QFontEngine::dc() const
{
    return hdc;
}

void QFontEngine::getCMap()
{
}

void QFontEngine::getGlyphIndexes(const QChar *ch, int numChars, glyph_t *glyphs, bool mirrored) const
{
    if (mirrored) {
        while(numChars--) {
            *glyphs = ::mirroredChar(*ch).unicode();
            glyphs++;
            ch++;
        }
    } else {
        while(numChars--) {
            *glyphs = ch->unicode();
            glyphs++;
            ch++;
        }
    }
}


// non Uniscribe engine ---------------------------------------------

QFontEngineWin::QFontEngineWin(const char * name, HDC _hdc, HFONT _hfont, bool stockFont, LOGFONT lf)
{
    //qDebug("regular windows font engine created: font='%s', size=%d", name, lf.lfHeight);

    _name = name;

    hdc = _hdc;
    hfont = _hfont;
    logfont = lf;
    SelectObject(hdc, hfont);
    stockFont = stockFont;

    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    BOOL res = GetTextMetricsW(dc(), &tm.w);
#ifndef QT_NO_DEBUG
    if (!res)
        qCritical("QFontEngineWin: GetTextMetrics failed (%s)", qt_error_string().local8Bit());
#endif
    cache_cost = tm.w.tmHeight * tm.w.tmAveCharWidth * 2000;
    memset(widthCache, 0, sizeof(widthCache));
}


QFontEngine::Error QFontEngineWin::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, bool mirrored) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return OutOfMemory;
    }

    getGlyphIndexes(str, len, glyphs, mirrored);

    HDC hdc = dc();
    unsigned int glyph;
    for(register int i = 0; i < len; i++) {
        glyph = *(glyphs + i);
        glyphs[i].advance = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
        // font-width cache failed
        if (!glyphs[i].advance) {
            SIZE size = {0, 0};
            GetTextExtentPoint32W(hdc, (wchar_t *)str, 1, &size);
            glyphs[i].advance = size.cx;
            // if glyph's within cache range, store it for later
            if (glyph < widthCacheSize && size.cx > 0 && size.cx < 0x100)
                ((QFontEngineWin *)this)->widthCache[glyph] = size.cx;
        }
        str++;
    }

    *nglyphs = len;
    return NoError;
}

#define COLOR_VALUE(c) ((p->flags & QPainter::RGBColor) ? RGB(c.red(),c.green(),c.blue()) : c.pixel())


void QFontEngineWin::draw(QPainter *p, int x, int y, const QTextItemInt &si)
{
    bool force_bitmap = p->rop != QPainter::CopyROP;
    force_bitmap |= p->txop >= QPainterPrivate::TxScale
                    && !(tm.w.tmPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE));

    double scale = 1.;
    int angle = 0;
    bool transform = false;

    if (force_bitmap || p->txop >= QPainterPrivate::TxScale) {
        // Draw rotated and sheared text on CE

        // All versions can draw rotated text natively. Scaling can be done with window/viewport transformations
        // the hard part is only shearing

        if (force_bitmap || p->m11() != p->m22() || p->m12() != -p->m21()) {
            // shearing transformation, have to do the work by hand
            QRect bbox(0, 0, si.width, si.ascent + si.descent + 1);
            int w=bbox.width(), h=bbox.height();
            int aw = w, ah = h;
            int tx=-bbox.x(), ty=-bbox.y();    // text position
            QMatrix mat1 = p->xmat;
            if (aw == 0 || ah == 0)
                return;
            double rx = (double)w / (double)aw;
            double ry = (double)h / (double)ah;
            QMatrix mat2 = QPixmap::trueMatrix(QMatrix(rx, 0, 0, ry, 0, 0)*mat1, aw, ah);

            QBitmap bm(aw, ah, true);
            QPainter paint;
            paint.begin(&bm);             // draw text in bitmap
            HDC oldDC = hdc;
            hdc = paint.handle();
            SelectObject(hdc, hfont);
            draw(&paint, 0, si.ascent, si);
            hdc = oldDC;
            paint.end();
            QBitmap wx_bm = bm.xForm(mat2); // transform bitmap
            if (wx_bm.isNull())
                return;

            double fx=x, fy = y - si.ascent, nfx, nfy;
            mat1.map(fx,fy, &nfx,&nfy);
            double tfx=tx, tfy=ty, dx, dy;
            mat2.map(tfx, tfy, &dx, &dy);     // compute position of bitmap
            x = qRound(nfx-dx);
            y = qRound(nfy-dy);
#if 0
            if (p->testf(QPainter::ExtDev)) {                // to printer
                QRegion reg(*wx_bm);
                reg.translate(x, y);
                HBRUSH brush = CreateSolidBrush(COLOR_VALUE(p->cpen.data->color));
                FillRgn(hdc, reg.handle(), brush);
                DeleteObject(brush);
            } else
#endif
            {                                // to screen/pixmap
                // this code is also used in bitBlt() in qpaintdevice_win.cpp
                // (for the case that you have a selfmask)
                const DWORD ropCodes[] = {
                    0x00b8074a, // PSDPxax,  CopyROP,
                    0x00ba0b09, // DPSnao,   OrROP,
                    0x009a0709, // DPSnax,   XorROP,
                    0x008a0e06, // DSPnoa,   EraseROP=NotAndROP,
                    0x008b0666, // DSPDxoxn, NotCopyROP,
                    0x00ab0889, // DPSono,   NotOrROP,
                    0x00a90189, // DPSoxn,   NotXorROP,
                    0x00a803a9, // DPSoa,    NotEraseROP=AndROP,
                    0x00990066, // DSxn,     NotROP,
                    0x008800c6, // DSa,      ClearROP,
                    0x00bb0226, // DSno,     SetROP,
                    0x00aa0029, // D,        NopROP,
                    0x00981888, // SDPSonoxn,AndNotROP,
                    0x00b906e6, // DSPDaoxn, OrNotROP,
                    0x009b07a8, // SDPSoaxn, NandROP,
                    0x00891b08  // SDPSnaoxn,NorROP,
                };
                HBRUSH b = CreateSolidBrush(COLOR_VALUE(p->cpen.data->color));
                COLORREF tc, bc;
                b = (HBRUSH)SelectObject(hdc, b);
                tc = SetTextColor(hdc, COLOR_VALUE(Qt::black));
                bc = SetBkColor(hdc, COLOR_VALUE(Qt::white));
                HDC wx_dc;
                int wx_sy;
                if (wx_bm.isMultiCellPixmap()) {
                    wx_dc = wx_bm.multiCellHandle();
                    wx_sy = wx_bm.multiCellOffset();
                } else {
                    wx_dc = wx_bm.handle();
                    wx_sy = 0;
                }
                BitBlt(hdc, x, y, wx_bm.width(), wx_bm.height(),
                        wx_dc, 0, wx_sy, ropCodes[p->rop]);
                SetBkColor(hdc, bc);
                SetTextColor(hdc, tc);
                DeleteObject(SelectObject(hdc, b));
            }
            return;
        }

        // rotation + scale + translation
        scale = sqrt(p->m11()*p->m22() - p->m12()*p->m21());
        angle = 1800*acos(p->m11()/scale)/M_PI;
        if (p->m12() < 0)
            angle = 3600 - angle;

        transform = true;
    } else if (p->txop == QPainterPrivate::TxTranslate) {
        p->map(x, y, &x, &y);
    }

    if (ti.flags & QTextItem::Underline || ti.flags & QTextItem::StrikeOut || scale != 1. || angle) {
        LOGFONT lf = logfont;
        lf.lfUnderline = (ti.flags & QTextItem::Underline);
        lf.lfStrikeOut = (ti.flags & QTextItem::StrikeOut);
        if (angle) {
            lf.lfOrientation = -angle;
            lf.lfEscapement = -angle;
        }
        if (scale != 1.) {
            lf.lfHeight = (int) (lf.lfHeight*scale);
            lf.lfWidth = (int) (lf.lfWidth*scale);
        }
        HFONT hf = QT_WA_INLINE(CreateFontIndirectW(&lf), CreateFontIndirectA((LOGFONTA*)&lf));
        SelectObject(hdc, hf);
    }

    unsigned int options =  0;

    QGlyphLayout *glyphs = si.glyphs;

#if 0
    // #### should move to the printer GC
    if(p->pdev->devType() == QInternal::Printer) {
        // some buggy printer drivers can't handle glyph indices correctly for latin1
        // If the string is pure latin1, we output the string directly, not the glyph indices.
        // There must be a better way to get this working, but currently I can't think of one.
        const QChar *uc = engine->string.unicode() + si.position;
        int l = engine->length(si - &engine->items[0]);
        int i = 0;
        bool latin = (l == si.num_glyphs);
        while (latin && i < l) {
            if(uc[i].unicode() >= 0x100)
                latin = false;
            ++i;
        }
        if(latin) {
            glyphs = (glyph_t *)uc;
            options = 0;
        }
    }
#endif

    int xo = x;

    y -= ascent();
    if (!(si.flags & QTextItem::RightToLeft)) {
        bool haveOffsets = false;
        int w = 0;
        for(int i = 0; i < si.num_glyphs; i++) {
            if (glyphs[i].offset.x || glyphs[i].offset.y) {
                haveOffsets = true;
                break;
            }
            w += glyphs[i].advance;
        }

        if (haveOffsets || transform) {
            for(int i = 0; i < si.num_glyphs; i++) {
                wchar_t chr = *glyphs;
                int xp = x + glyphs->offset.x;
                int yp = y + glyphs->offset.y;
                if (transform)
                    p->map(xp, yp, &xp, &yp);
                ExtTextOutW(hdc, xp, yp, options, 0, &chr, 1, 0);
                x += glyphs->advance;
            }
        } else {
            QVarLengthArray<wchar_t> g(si.num_glyphs);
            for (int i = 0; i < si.num_glyphs; ++i)
                g[i] = glyphs[i].glyph;
            // fast path
            ExtTextOutW(hdc, x + glyphs->offset.x, y + glyphs->offset.y, options, 0, g, si.num_glyphs, 0);
            x += w;
        }
    } else {
        glyphs += si.num_glyphs;
        for(int i = 0; i < si.num_glyphs; i++) {
            glyphs--;
            wchar_t chr = glyphs->glyph;
            int xp = x + glyphs->offset.x;
            int yp = y + glyphs->offset.y;
            if (transform)
                p->map(xp, yp, &xp, &yp);
            ExtTextOutW(hdc, xp, yp, options, 0, &chr, 1, 0);
            x += glyphs->advance;
        }
    }

    if (ti.flags & QTextItem::Underline || ti.flags & QTextItem::StrikeOut || scale != 1. || angle)
        DeleteObject(SelectObject(hdc, hfont));

    if (si.flags & QTextItem::Overline) {
        int lw = lineThickness();
        int yp = y - 1;
        Rectangle(hdc, xo, yp, x, yp + lw);

    }

}

glyph_metrics_t QFontEngineWin::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    int w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance;

    return glyph_metrics_t(0, -tm.w.tmAscent, w, tm.w.tmHeight, w, 0);
}

glyph_metrics_t QFontEngineWin::boundingBox(glyph_t glyph)
{
    SIZE s = {0, 0};
    WCHAR ch = glyph;
    BOOL res = GetTextExtentPoint32W(dc(), &ch, 1, &s);
    Q_UNUSED(res);
    return glyph_metrics_t(0, -tm.a.tmAscent, s.cx, tm.a.tmHeight, s.cx, 0);
}

int QFontEngineWin::ascent() const
{
    return tm.w.tmAscent;
}

int QFontEngineWin::descent() const
{
    return tm.w.tmDescent;
}

int QFontEngineWin::leading() const
{
    return tm.w.tmExternalLeading;
}

int QFontEngineWin::maxCharWidth() const
{
    return tm.w.tmMaxCharWidth;
}

int QFontEngineWin::minLeftBearing() const
{
    return 0;
}

int QFontEngineWin::minRightBearing() const
{
    return 0;
}

const char *QFontEngineWin::name() const
{
    return 0;
}

bool QFontEngineWin::canRender(const QChar *string, int len)
{
    while(len--) {
        if (tm.w.tmFirstChar > string->unicode() || tm.w.tmLastChar < string->unicode())
            return false;
    }
    return true;
}

QFontEngine::Type QFontEngineWin::type() const
{
    return QFontEngine::Win;
}


// box font engine --------------------------------------------------

QFontEngineBox::QFontEngineBox(int size)
    : _size(size)
{
    cache_cost = 1;
    hdc = GetDC(0);
    hfont = (HFONT)GetStockObject(SYSTEM_FONT);
    stockFont = true;
    paintDevice = false;
    ttf = false;

    cmap = 0;
    script_cache = 0;
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::Error QFontEngineBox::stringToCMap(const QChar *, int len, QGlyphLayout *glyphs, int *nglyphs, bool) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return OutOfMemory;
    }

    memset(glyphs, 0, len * sizeof(glyph_t));
    *nglyphs = len;

    for (int i = 0; i < len; ++i)
        glyphs[i].advance = _size;

    return NoError;
}

void QFontEngineBox::draw(QPainter *p, int x, int y, const QTextItemInt &si)
{
    Q_UNUSED(p);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(engine);
    Q_UNUSED(si);
}

glyph_metrics_t QFontEngineBox::boundingBox(const QGlyphLayout *, int numGlyphs)
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

glyph_metrics_t QFontEngineBox::boundingBox(glyph_t)
{
    return glyph_metrics_t(0, _size, _size, _size, _size, 0);
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
    int l = qRound(_size * 0.15);
    return (l > 0) ? l : 1;
}

int QFontEngineBox::maxCharWidth() const
{
    return _size;
}

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender(const QChar *, int)
{
    return true;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}
