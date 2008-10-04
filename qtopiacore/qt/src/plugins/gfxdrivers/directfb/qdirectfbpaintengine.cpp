/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qdirectfbpaintengine.h"

#ifndef QT_NO_DIRECTFB

#include "qdirectfbsurface.h"
#include "qdirectfbscreen.h"
#include "qdirectfbpixmap.h"
#include <directfb.h>
#include <qmatrix.h>
#include <qvarlengtharray.h>
#include <qcache.h>
#include <qmath.h>
#include <private/qpixmapdata_p.h>

static inline uint ALPHA_MUL(uint x, uint a)
{
    uint t = x * a;
    t = ((t + (t >> 8) + 0x80) >> 8) & 0xff;
    return t;
}

class SurfaceCache
{
public:
    SurfaceCache(IDirectFB *fb);
    ~SurfaceCache();

    inline IDirectFBSurface *getSurface(const uint *buffer, int size);
    inline void clear();

private:
    IDirectFB *fb;
    IDirectFBSurface *surface;
    uint *buffer;
    int bufsize;
};

SurfaceCache::SurfaceCache(IDirectFB *dfb)
    : fb(dfb), surface(0), buffer(0), bufsize(0)
{
}

class CachedImage
{
public:
    CachedImage(const QImage &image);
    ~CachedImage();

    IDirectFBSurface *surface() { return s; }

private:
    IDirectFBSurface *s;
};

CachedImage::CachedImage(const QImage &image)
    : s(0)
{
    IDirectFBSurface *tmpSurface = 0;
    DFBSurfaceDescription description;
    description = QDirectFBScreen::getSurfaceDescription(image);
    IDirectFB *fb = QDirectFBScreen::instance()->dfb();

    DFBResult result = fb->CreateSurface(fb, &description, &tmpSurface);
    if (result != DFB_OK) {
        DirectFBError("CachedImage CreateSurface", result);
        return;
    }
#ifndef QT_NO_DIRECTFB_PALETTE
    QDirectFBScreen::setSurfaceColorTable(tmpSurface, image);
#endif

    description.flags = DFBSurfaceDescriptionFlags(description.flags ^ DSDESC_PREALLOCATED);
    result = fb->CreateSurface(fb, &description, &s);
    if (result != DFB_OK)
        DirectFBError("QDirectFBPaintEngine failed caching image", result);

#ifndef QT_NO_DIRECTFB_PALETTE
    QDirectFBScreen::setSurfaceColorTable(s, image);
#endif

    if (s) {
        s->SetBlittingFlags(s, DSBLIT_NOFX);
        s->Blit(s, tmpSurface, 0, 0, 0);
    }
    if (tmpSurface)
        tmpSurface->Release(tmpSurface);
}

CachedImage::~CachedImage()
{
    if (s)
        s->Release(s);
}

static QCache<qint64, CachedImage> imageCache(4*1024*1024); // 4 MB

IDirectFBSurface* SurfaceCache::getSurface(const uint *buf, int size)
{
    if (buffer == buf && bufsize == size)
        return surface;

    clear();

    DFBSurfaceDescription description;
    description = QDirectFBScreen::getSurfaceDescription(buf, size);

    DFBResult result = fb->CreateSurface(fb, &description, &surface);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPaintEngine: SurfaceCache: "
                      "Unable to create surface", result);
    }
    buffer = const_cast<uint*>(buf);
    bufsize = size;

    return surface;
}

void SurfaceCache::clear()
{
    if (surface)
        surface->Release(surface);
    surface = 0;
    buffer = 0;
    bufsize = 0;
}

SurfaceCache::~SurfaceCache()
{
    clear();
}

class QDirectFBPaintEnginePrivate : public QPaintEnginePrivate
{
public:
    QDirectFBPaintEnginePrivate(QDirectFBPaintEngine *p);
    ~QDirectFBPaintEnginePrivate();

    IDirectFBSurface *surface;

    QPen pen;
    QBrush brush;

    bool simpleClip;
    bool clipEnabled;
    bool antialiased;

    bool simplePen;
    bool simpleBrush;

    bool matrixRotShear;
    bool matrixScale;

    void setMatrix(const QMatrix &m);
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setBackground(const QBrush &brush);
    void setCompositionMode(QPainter::CompositionMode mode);
    void setOpacity(const qreal value);

    inline void usePen() const;
    inline void useBrush() const;

    inline bool lock();
    inline void unlock();

    inline bool isSimpleClip(const QRect &rect) const;
    inline bool isSimpleClip(const QRectF &rect) const;

    void drawLines(const QLine *lines, int count) const;
    void drawLines(const QLineF *lines, int count) const;

    void fillRegion(const QRegion &r) const;
    void fillRects(const QRect *rects, int count) const;
    void drawRects(const QRect *rects, int count) const;
    void fillRects(const QRectF *rects, int count) const;
    void drawRects(const QRectF *rects, int count) const;

    void drawPixmap(const QRectF &dest,
                    const QPixmap &pixmap, const QRectF &src);
    void drawTiledPixmap(const QRectF &dest,
                         const QPixmap &pixmap, const QPointF &src);
    void drawImage(const QRectF &dest, const QImage &image, const QRectF &src);

    void setClip(const QRegion &region);
    void updateClip();
    void updateFlags();

    void begin(QPaintDevice *device);
    void end();

    SurfaceCache *surfaceCache;

private:
    QRegion rectsToClippedRegion(const QRect *rects, int n) const;
    QRegion rectsToClippedRegion(const QRectF *rects, int n) const;

    IDirectFB *fb;
    DFBSurfaceDescription fbDescription;
    int fbWidth;
    int fbHeight;

    QBrush bgBrush;
    QRegion clip;
    quint8 opacity;
    QMatrix matrix;

    quint32 drawFlags;
    quint32 blitFlags;
    quint32 duffFlags;
    bool dirtyFlags;

    QDirectFBPaintEngine *q;
};

QDirectFBPaintEnginePrivate::QDirectFBPaintEnginePrivate(QDirectFBPaintEngine* p)
    : surface(0), q(p)
{
    fb = QDirectFBScreen::instance()->dfb();
    surfaceCache = new SurfaceCache(fb);
    static int cacheLimit = qgetenv("QT_DIRECTFB_IMAGECACHE").toInt();
    if (cacheLimit > 0)
        imageCache.setMaxCost(cacheLimit * 1024);
}

QDirectFBPaintEnginePrivate::~QDirectFBPaintEnginePrivate()
{
    unlock();
    delete surfaceCache;
}

bool QDirectFBPaintEnginePrivate::isSimpleClip(const QRect &rect) const
{
    if (simpleClip)
        return true;

    if (clip.numRects() <= 1) // clipPath
        return false;

    return qt_region_strictContains(clip, matrix.mapRect(rect));
}

bool QDirectFBPaintEnginePrivate::isSimpleClip(const QRectF &rect) const
{
    if (simpleClip)
        return true;

    if (clip.numRects() <= 1) // clipPath
        return false;

    return qt_region_strictContains(clip, matrix.mapRect(rect).toRect());
}

bool QDirectFBPaintEnginePrivate::lock()
{
    return q->prepare(q->paintDevice());
}

void QDirectFBPaintEnginePrivate::unlock()
{
    QPaintDevice *device = q->paintDevice();
    switch (device->devType()) {
    case QInternal::CustomRaster: {
        QDirectFBSurface *s = static_cast<QDirectFBSurface*>(device);
        s->unlockDirectFB();
        break;
    }
    case QInternal::Pixmap: {
        QPixmap *pixmap = static_cast<QPixmap*>(device);
        QPixmapData *data = pixmap->pixmapData();
        QDirectFBPixmapData *dfbData = static_cast<QDirectFBPixmapData*>(data);
        dfbData->unlockDirectFB();
        break;
    }
    default:
        break;
    }
}

void QDirectFBPaintEnginePrivate::setMatrix(const QMatrix &m)
{
    matrix = m;
    matrixRotShear = (matrix.m12() != 0 || matrix.m21() != 0);
    matrixScale = (matrix.m11() != 1 || matrix.m22() != 1);
}

void QDirectFBPaintEnginePrivate::begin(QPaintDevice *device)
{
    if (device->devType() == QInternal::CustomRaster) {
        QDirectFBSurface *win = static_cast<QDirectFBSurface*>(device);
        surface = win->surface();
    } else if (device->devType() == QInternal::Pixmap) {
        QPixmap *pixmap = static_cast<QPixmap*>(device);
        QPixmapData *data = pixmap->pixmapData();
        if (data->classId() == QPixmapData::DirectFBClass) {
            QDirectFBPixmapData *dfbData = static_cast<QDirectFBPixmapData*>(data);
            surface = dfbData->dfbSurface();
        }
    }

    if (!surface) {
        qFatal("QDirectFBPaintEngine used on an invalid device: 0x%x",
               device->devType());
    }

    surface->GetSize(surface, &fbWidth, &fbHeight);

    setMatrix(QMatrix());
    antialiased = false;
    drawFlags = DSDRAW_BLEND;
    blitFlags = DSBLIT_BLEND_ALPHACHANNEL;
    duffFlags = DSPD_SRC_OVER;
    opacity = 255;
    dirtyFlags = true;
    setClip(QRect(0, 0, fbWidth, fbHeight));
}

void QDirectFBPaintEnginePrivate::end()
{
    surface->SetClip(surface, NULL);
    surface = 0;
}

void QDirectFBPaintEnginePrivate::setBackground(const QBrush &brush)
{
    bgBrush = brush;
}

void QDirectFBPaintEnginePrivate::setPen(const QPen &p)
{
    pen = p;
    simplePen = (pen.style() == Qt::NoPen) ||
                pen.style() == Qt::SolidLine && !antialiased
                && (pen.widthF() == 0 || pen.widthF() <= 1 && !matrixScale);
}

void QDirectFBPaintEnginePrivate::setBrush(const QBrush &b)
{
    // TODO: accelerate texture pattern
    brush = b;
    simpleBrush = (brush.style() == Qt::NoBrush) ||
                  brush.style() == Qt::SolidPattern && !antialiased;
}

void QDirectFBPaintEnginePrivate::setCompositionMode(QPainter::CompositionMode mode)
{
    drawFlags &= ~(DSDRAW_XOR);
    blitFlags &= ~(DSBLIT_XOR);

    // TODO: check these mappings!!!!
    quint32 duff = DSPD_NONE;
    quint32 blit = blitFlags;

    switch (mode) {
    case QPainter::CompositionMode_SourceOver:
        duff = DSPD_SRC_OVER;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_DestinationOver:
        duff = DSPD_DST_OVER;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_Clear:
        duff = DSPD_CLEAR;
        blit ^= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_Source:
        duff = DSPD_SRC;
        blit ^= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_Destination:
        blit ^= DSBLIT_BLEND_ALPHACHANNEL;
        return;
    case QPainter::CompositionMode_SourceIn:
        duff = DSPD_SRC_IN;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_DestinationIn:
        duff = DSPD_DST_IN;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_SourceOut:
        duff = DSPD_SRC_OUT;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_DestinationOut:
        duff = DSPD_DST_OUT;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_SourceAtop:
        duff = DSPD_SRC_OVER;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        break;
    case QPainter::CompositionMode_DestinationAtop:
        duff = DSPD_DST_OVER;
        break;
    case QPainter::CompositionMode_Xor:
        duff = DSPD_NONE;
        blit |= DSBLIT_BLEND_ALPHACHANNEL;
        drawFlags |= DSDRAW_XOR;
        blit |= DSBLIT_XOR;
        dirtyFlags = true;
        break;
    default:
        qWarning("QDirectFBPaintEnginePrivate::setCompositionMode(): "
                 "mode %d not implemented", mode);
        break;
    }

    if (duff != duffFlags || blit != blitFlags) {
        duffFlags = duff;
        blitFlags = blit;
        dirtyFlags = true;
    }
}

void QDirectFBPaintEnginePrivate::setOpacity(const qreal value)
{
    const bool wasOpaque = (opacity == 255);
    opacity = quint8(value * 255);
    const bool opaque = (opacity == 255);

    if (opaque == wasOpaque)
        return;

    if (opaque)
        blitFlags ^= (DSBLIT_BLEND_COLORALPHA | DSBLIT_SRC_PREMULTCOLOR);
    else
        blitFlags |= (DSBLIT_BLEND_COLORALPHA | DSBLIT_SRC_PREMULTCOLOR);

    dirtyFlags = true;
}

void QDirectFBPaintEnginePrivate::updateFlags()
{
    if (!dirtyFlags)
        return;
    surface->SetDrawingFlags(surface, DFBSurfaceDrawingFlags(drawFlags));
    surface->SetBlittingFlags(surface, DFBSurfaceBlittingFlags(blitFlags));
    surface->SetPorterDuff(surface, DFBSurfacePorterDuffRule(duffFlags));
}

void QDirectFBPaintEnginePrivate::usePen() const
{
    const QColor color = pen.color();
    const quint8 alpha = (opacity == 255 ?
                          color.alpha() : ALPHA_MUL(color.alpha(), opacity));
    surface->SetColor(surface,
                      color.red(), color.green(), color.blue(), alpha);
}

void QDirectFBPaintEnginePrivate::useBrush() const
{
    const QColor color = brush.color();
    const quint8 alpha = (opacity == 255 ?
                          color.alpha() : ALPHA_MUL(color.alpha(), opacity));
    surface->SetColor(surface,
                      color.red(), color.green(), color.blue(), alpha);
}

void QDirectFBPaintEnginePrivate::drawLines(const QLine *lines, int n) const
{
    QVarLengthArray<DFBRegion> regions(n);

    for (int i = 0; i < n; ++i) {
        const QLine l = matrix.map(lines[i]);

        // TODO: clip!

        regions[i].x1 = l.x1();
        regions[i].y1 = l.y1();
        regions[i].x2 = l.x2();
        regions[i].y2 = l.y2();
    }
    surface->DrawLines(surface, regions.data(), n);
}

void QDirectFBPaintEnginePrivate::drawLines(const QLineF *lines, int n) const
{
    QVarLengthArray<DFBRegion> regions(n);

    for (int i = 0; i < n; ++i) {
        const QLine l = matrix.map(lines[i]).toLine();

        // TODO: clip!

        regions[i].x1 = l.x1();
        regions[i].y1 = l.y1();
        regions[i].x2 = l.x2();
        regions[i].y2 = l.y2();
    }
    surface->DrawLines(surface, regions.data(), n);
}

QRegion QDirectFBPaintEnginePrivate::rectsToClippedRegion(const QRect *rects,
                                                          int n) const
{
    QRegion region;

    for (int i = 0; i < n; ++i) {
        const QRect r = matrix.mapRect(rects[i]);
        region += clip & r;
    }

    return region;
}

QRegion QDirectFBPaintEnginePrivate::rectsToClippedRegion(const QRectF *rects,
                                                          int n) const
{
    QRegion region;

    for (int i = 0; i < n; ++i) {
        const QRect r = matrix.mapRect(rects[i]).toRect();
        region += clip & r;
    }

    return region;
}

void QDirectFBPaintEnginePrivate::fillRegion(const QRegion &region) const
{
    const QVector<QRect> rects = region.rects();
    const int n = rects.size();
    QVarLengthArray<DFBRectangle> dfbRects(n);

    for (int i = 0; i < n; ++i) {
        const QRect r = rects.at(i);
        dfbRects[i].x = r.x();
        dfbRects[i].y = r.y();
        dfbRects[i].w = r.width();
        dfbRects[i].h = r.height();

    }
    surface->FillRectangles(surface, dfbRects.data(), n);
}

void QDirectFBPaintEnginePrivate::fillRects(const QRect *rects, int n) const
{
    const QRegion region = rectsToClippedRegion(rects, n);
    fillRegion(region);
}

void QDirectFBPaintEnginePrivate::fillRects(const QRectF *rects, int n) const
{
    const QRegion region = rectsToClippedRegion(rects, n);
    fillRegion(region);
}

void QDirectFBPaintEnginePrivate::drawRects(const QRect *rects, int n) const
{
    for (int i = 0; i < n; ++i) {
        const QRect r = matrix.mapRect(rects[i]);
        surface->DrawRectangle(surface, r.x(), r.y(), r.width(), r.height());
    }
}

void QDirectFBPaintEnginePrivate::drawRects(const QRectF *rects, int n) const
{
    for (int i = 0; i < n; ++i) {
        const QRect r = matrix.mapRect(rects[i]).toRect();
        surface->DrawRectangle(surface, r.x(), r.y(), r.width(), r.height());
    }
}

void QDirectFBPaintEnginePrivate::drawPixmap(const QRectF &dest,
                                             const QPixmap &pixmap,
                                             const QRectF &src)
{
    surface->SetColor(surface, 0xff, 0xff, 0xff, opacity);

    const bool changeFlags = !pixmap.hasAlphaChannel()
                             && (blitFlags & DSBLIT_BLEND_ALPHACHANNEL);
    if (changeFlags) {
        quint32 flags = blitFlags ^ DSBLIT_BLEND_ALPHACHANNEL;
        surface->SetBlittingFlags(surface, DFBSurfaceBlittingFlags(flags));
    }

    QPixmapData *data = pixmap.pixmapData();
    Q_ASSERT(data->classId() == QPixmapData::DirectFBClass);
    QDirectFBPixmapData *dfbData = static_cast<QDirectFBPixmapData*>(data);
    IDirectFBSurface *s = dfbData->dfbSurface();
    const QRect sr = src.toRect();
    const QRect dr = matrix.mapRect(dest).toRect();
    const DFBRectangle sRect = { sr.x(), sr.y(), sr.width(), sr.height() };
    DFBResult result;

    if (dr.size() == sr.size()) {
        result = surface->Blit(surface, s, &sRect, dr.x(), dr.y());
    } else {
        const DFBRectangle dRect = { dr.x(), dr.y(), dr.width(), dr.height() };
        result = surface->StretchBlit(surface, s, &sRect, &dRect);
    }
    if (result != DFB_OK)
        DirectFBError("QDirectFBPaintEngine::drawPixmap()", result);
    if (changeFlags)
        surface->SetBlittingFlags(surface, DFBSurfaceBlittingFlags(blitFlags));
}

void QDirectFBPaintEnginePrivate::drawTiledPixmap(const QRectF &dest,
                                                  const QPixmap &pixmap,
                                                  const QPointF &src)
{
    Q_ASSERT(src.isNull());

    surface->SetColor(surface, 0xff, 0xff, 0xff, opacity);

    const bool changeFlags = !pixmap.hasAlphaChannel()
                             && (blitFlags & DSBLIT_BLEND_ALPHACHANNEL);
    if (changeFlags) {
        quint32 flags = blitFlags ^ DSBLIT_BLEND_ALPHACHANNEL;
        surface->SetBlittingFlags(surface, DFBSurfaceBlittingFlags(flags));
    }

    QPixmapData *data = pixmap.pixmapData();
    Q_ASSERT(data->classId() == QPixmapData::DirectFBClass);
    QDirectFBPixmapData *dfbData = static_cast<QDirectFBPixmapData*>(data);
    IDirectFBSurface *s = dfbData->dfbSurface();
    const QRect dr = matrix.mapRect(dest).toRect();
    DFBResult result = DFB_OK;

    if (!matrixScale && dr == QRect(0, 0, fbWidth, fbHeight)) {
        result = surface->TileBlit(surface, s, 0, 0, 0);
    } else if (!matrixScale) {
        const int dx = pixmap.width();
        const int dy = pixmap.height();
        const DFBRectangle rect = { 0, 0, dx, dy };
        QVarLengthArray<DFBRectangle> rects;
        QVarLengthArray<DFBPoint> points;

        for (int y = dr.y(); y <= dr.bottom(); y += dy) {
            for (int x = dr.x(); x <= dr.right(); x += dx) {
                rects.append(rect);
                const DFBPoint point = { x, y };
                points.append(point);
            }
        }
        result = surface->BatchBlit(surface, s, rects.constData(),
                                    points.constData(), points.size());
    } else {
        const QRect sr = matrix.mapRect(QRect(0, 0, pixmap.width(), pixmap.height()));
        const int dx = sr.width();
        const int dy = sr.height();
        const DFBRectangle sRect = { 0, 0, dx, dy };

        for (int y = dr.y(); y <= dr.bottom(); y += dy) {
            for (int x = dr.x(); x <= dr.right(); x += dx) {
                const DFBRectangle dRect = { x, y, dx, dy };
                result = surface->StretchBlit(surface, s, &sRect, &dRect);
                if (result != DFB_OK) {
                    y = dr.bottom() + 1;
                    break;
                }
            }
        }
    }

    if (result != DFB_OK)
        DirectFBError("QDirectFBPaintEngine::drawTiledPixmap()", result);

    if (changeFlags)
        surface->SetBlittingFlags(surface, DFBSurfaceBlittingFlags(blitFlags));
}

void QDirectFBPaintEnginePrivate::drawImage(const QRectF &dest,
                                            const QImage &srcImage,
                                            const QRectF &src)
{
    QImage image = srcImage;
    if (QDirectFBScreen::getSurfacePixelFormat(image) == DSPF_UNKNOWN) {
        QImage::Format format;
        if (image.hasAlphaChannel())
            format = QImage::Format_ARGB32_Premultiplied;
        else
            format = QImage::Format_RGB32;
        image = image.convertToFormat(format);
    }

    CachedImage *img = imageCache[image.cacheKey()];
    IDirectFBSurface *imgSurface = 0;
    bool doRelease = false;

    if (img) {
        imgSurface = img->surface();
    } else {
        const int cost = image.width() * image.height() * image.depth() / 8;
        if (cost <= imageCache.maxCost()) {
            img = new CachedImage(image);
            imgSurface = img->surface();
            if (imgSurface) {
                imageCache.insert(image.cacheKey(), img, cost);
            } else {
                delete img;
                img = 0;
            }
        }

        if (!imgSurface) {
            DFBSurfaceDescription description;
            DFBResult result;

            description = QDirectFBScreen::getSurfaceDescription(image);
            result = fb->CreateSurface(fb, &description, &imgSurface);
            if (result != DFB_OK) {
                DirectFBError("QDirectFBPaintEnginePrivate::drawImage", result);
                return;
            }

#ifndef QT_NO_DIRECTFB_PALETTE
            QDirectFBScreen::setSurfaceColorTable(surface, image);
#endif
            doRelease = (imgSurface != 0);
        }
    }

    const QRect sr = src.toRect();
    const QRect dr = matrix.mapRect(dest).toRect();
    const DFBRectangle sRect = { sr.x(), sr.y(), sr.width(), sr.height() };

    surface->SetColor(surface, 0xff, 0xff, 0xff, opacity);

    const bool changeFlags = !image.hasAlphaChannel()
                             && (blitFlags & DSBLIT_BLEND_ALPHACHANNEL);
    if (changeFlags) {
        quint32 flags = blitFlags ^ DSBLIT_BLEND_ALPHACHANNEL;
        surface->SetBlittingFlags(surface, DFBSurfaceBlittingFlags(flags));
    }
    if (dr.size() == sr.size()) {
        surface->Blit(surface, imgSurface, &sRect, dr.x(), dr.y());
    } else {
        const DFBRectangle dRect = { dr.x(), dr.y(),
                                     dr.width(), dr.height() };
        surface->StretchBlit(surface, imgSurface, &sRect, &dRect);
    }
    if (changeFlags)
        surface->SetBlittingFlags(surface, DFBSurfaceBlittingFlags(blitFlags));
    if (doRelease)
        imgSurface->Release(imgSurface);
}

void QDirectFBPaintEnginePrivate::setClip(const QRegion &region)
{
    // TODO: cutoff for rotation

    if (region.isEmpty())
        clip = QRect(0, 0, fbWidth, fbHeight);
    else
        clip = matrix.map(region) & QRect(0, 0, fbWidth, fbHeight);
    clipEnabled = true;
    updateClip();
}

void QDirectFBPaintEnginePrivate::updateClip()
{
    QRegion clipRegion = QRect(0, 0, fbWidth, fbHeight);

    if (!q->systemClip().isEmpty())
        clipRegion &= q->systemClip();
    if (clipEnabled)
        clipRegion &= clip;

    simpleClip = (clipRegion.rects().size() <= 1);

    const QRect clipRect = clipRegion.boundingRect();
    const DFBRegion r = { clipRect.x(), clipRect.y(),
                          clipRect.x() + clipRect.width(),
                          clipRect.y() + clipRect.height() };

    surface->SetClip(surface, &r);
}

QDirectFBPaintEngine::QDirectFBPaintEngine()
    : QRasterPaintEngine()
{
    d = new QDirectFBPaintEnginePrivate(this);
}

QDirectFBPaintEngine::~QDirectFBPaintEngine()
{
    delete d;
}

bool QDirectFBPaintEngine::begin(QPaintDevice *device)
{
    d->begin(device);
    const bool status = QRasterPaintEngine::begin(device);

    // XXX: QRasterPaintEngine::begin() resets the capabilities
    gccaps |= PorterDuff;

    return status;
}

bool QDirectFBPaintEngine::end()
{
    d->end();
    return QRasterPaintEngine::end();
}

void QDirectFBPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyTransform)
        d->setMatrix(state.matrix());

#if 0
    if (flags & DirtyBackgroundMode) {
        qWarning("DirtyBackgroundMode not implemented");
    }
#endif

    if (flags & DirtyBackground)
        d->setBackground(state.backgroundBrush());

    if (flags & DirtyPen)
        d->setPen(state.pen());

    if (flags & DirtyBrush)
        d->setBrush(state.brush());

#if 0
    if (flags & DirtyBrushOrigin)
        qWarning("DirtyBrushOrigin not implemented");
#endif

    if (flags & DirtyClipRegion)
        d->setClip(state.clipRegion());

    if (flags & DirtyClipEnabled) {
        d->clipEnabled = state.isClipEnabled();
        d->updateClip();
    }

    if (flags & DirtyClipPath) {
        d->setClip(QRegion());
        d->simpleClip = false;
    }

    if (flags & DirtyCompositionMode)
        d->setCompositionMode(state.compositionMode());

    if (flags & DirtyOpacity)
        d->setOpacity(state.opacity());

    if (flags & DirtyHints)
        d->antialiased = bool(state.renderHints() & QPainter::Antialiasing);

    d->updateFlags();

    QRasterPaintEngine::updateState(state);
}

void QDirectFBPaintEngine::drawRects(const QRect  *rects, int rectCount)
{
    if (!d->simpleClip || d->matrixRotShear || !d->simpleBrush || !d->simplePen) {
        d->lock();
        QRasterPaintEngine::drawRects(rects, rectCount);
        return;
    }

    d->unlock();

    if (d->brush != Qt::NoBrush) {
        d->useBrush();
        d->fillRects(rects, rectCount);
    }
    if (d->pen != Qt::NoPen) {
        d->usePen();
        d->drawRects(rects, rectCount);
    }
}

void QDirectFBPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    if (!d->simpleClip || d->matrixRotShear || !d->simpleBrush || !d->simplePen) {
        d->lock();
        QRasterPaintEngine::drawRects(rects, rectCount);
        return;
    }

    d->unlock();

    if (d->brush != Qt::NoBrush) {
        d->useBrush();
        d->fillRects(rects, rectCount);
    }
    if (d->pen != Qt::NoPen) {
        d->usePen();
        d->drawRects(rects, rectCount);
    }
}

void QDirectFBPaintEngine::drawLines(const QLine *lines, int lineCount)
{
    if (!d->simplePen || !d->simpleClip) {
        d->lock();
        QRasterPaintEngine::drawLines(lines, lineCount);
        return;
    }

    if (d->pen != Qt::NoPen) {
        d->unlock();
        d->usePen();
        d->drawLines(lines, lineCount);
    }
}

void QDirectFBPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    if (!d->simplePen || !d->simpleClip) {
        d->lock();
        QRasterPaintEngine::drawLines(lines, lineCount);
        return;
    }

    if (d->pen != Qt::NoPen) {
        d->unlock();
        d->usePen();
        d->drawLines(lines, lineCount);
    }
}

void QDirectFBPaintEngine::drawImage(const QRectF &r, const QImage &image,
                                     const QRectF &sr,
                                     Qt::ImageConversionFlags flags)
{
    Q_UNUSED(flags); // XXX

#ifndef QT_NO_DIRECTFB_PREALLOCATED
    if (!d->isSimpleClip(r) || d->matrixRotShear)
#endif
    {
        d->lock();
        QRasterPaintEngine::drawImage(r, image, sr, flags);
        return;
    }

#ifndef QT_NO_DIRECTFB_PREALLOCATED
    d->unlock();
    d->drawImage(r, image, sr);
#endif
}

void QDirectFBPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap,
                                      const QRectF &sr)
{
    if (!d->isSimpleClip(r) || d->matrixRotShear
        || pixmap.pixmapData()->classId() != QPixmapData::DirectFBClass)
    {
        d->lock();
        QRasterPaintEngine::drawPixmap(r, pixmap, sr);
        return;
    }

    d->unlock();
    d->drawPixmap(r, pixmap, sr);
}

void QDirectFBPaintEngine::drawTiledPixmap(const QRectF &r,
                                           const QPixmap &pixmap,
                                           const QPointF &sp)
{
    if (!d->isSimpleClip(r) || d->matrixRotShear || !sp.isNull()) {
        d->lock();
        QRasterPaintEngine::drawTiledPixmap(r, pixmap, sp);
        return;
    }

    d->unlock();
    d->drawTiledPixmap(r, pixmap, sp);
}

void QDirectFBPaintEngine::drawPath(const QPainterPath &path)
{
    d->lock();
    QRasterPaintEngine::drawPath(path);
}

void QDirectFBPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    d->lock();
    QRasterPaintEngine::drawPoints(points, pointCount);
}

void QDirectFBPaintEngine::drawPoints(const QPoint *points, int pointCount)
{
    d->lock();
    QRasterPaintEngine::drawPoints(points, pointCount);
}

void QDirectFBPaintEngine::drawPolygon(const QPointF *points, int pointCount,
                                       PolygonDrawMode mode)
{
    d->lock();
    QRasterPaintEngine::drawPolygon(points, pointCount, mode);
}

void QDirectFBPaintEngine::drawPolygon(const QPoint *points, int pointCount,
                                       PolygonDrawMode mode)
{
    d->lock();
    QRasterPaintEngine::drawPolygon(points, pointCount, mode);
}

void QDirectFBPaintEngine::drawTextItem(const QPointF &p,
                                        const QTextItem &textItem)
{
    d->lock();
    QRasterPaintEngine::drawTextItem(p, textItem);
}

void QDirectFBPaintEngine::drawColorSpans(const QSpan *spans, int count,
                                          uint color)
{
    color = INV_PREMUL(color);

    QVarLengthArray<DFBRegion> lines(count);
    int j = 0;
    for (int i = 0; i < count; ++i) {
        if (spans[i].coverage == 255) {
            lines[j].x1 = spans[i].x;
            lines[j].y1 = spans[i].y;
            lines[j].x2 = spans[i].x + spans[i].len - 1;
            lines[j].y2 = spans[i].y;
            ++j;
        } else {
            DFBSpan span = { spans[i].x, spans[i].len };
            uint c = BYTE_MUL(color, spans[i].coverage);
            d->surface->SetColor(d->surface,
                                 qRed(c), qGreen(c), qBlue(c), qAlpha(c));
            d->surface->FillSpans(d->surface, spans[i].y, &span, 1);
        }
    }
    if (j > 0) {
        d->surface->SetColor(d->surface,
                             qRed(color), qGreen(color), qBlue(color),
                             qAlpha(color));
        d->surface->DrawLines(d->surface, lines.data(), j);
    }
}

void QDirectFBPaintEngine::drawBufferSpan(const uint *buffer, int bufsize,
                                          int x, int y, int length,
                                          uint const_alpha)
{
    IDirectFBSurface *src = d->surfaceCache->getSurface(buffer, bufsize);
    src->SetColor(src, 0, 0, 0, const_alpha);
    const DFBRectangle rect = { 0, 0, length, 1 };
    d->surface->Blit(d->surface, src, &rect, x, y);
}

#endif // QT_NO_DIRECTFB
