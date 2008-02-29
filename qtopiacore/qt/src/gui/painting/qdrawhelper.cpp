/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qdrawhelper_p.h>
#include <private/qpaintengine_raster_p.h>
#include <private/qpainter_p.h>
#include <private/qmath_p.h>
#include <math.h>
#define MASK(src, a) src = BYTE_MUL(src, a)

#if defined(Q_OS_IRIX) && defined(Q_CC_GNU) && __GNUC__ == 3 && __GNUC__ < 4 && QT_POINTER_SIZE == 8
#define Q_IRIX_GCC3_3_WORKAROUND
//
// work around http://gcc.gnu.org/bugzilla/show_bug.cgi?id=14484
//
static uint gccBug(uint value) __attribute__((noinline));
static uint gccBug(uint value)
{
    return value;
}
#endif


#ifdef Q_WS_QWS
#define QWS_CALLBACK_IF_NONBUFFERED(spandata, bufferedCode, callbackCode)  \
if ((spandata)->rasterBuffer->buffer())                                    \
    bufferedCode                                                           \
else                                                                       \
    callbackCode
#else
#define QWS_CALLBACK_IF_NONBUFFERED(spandata, bufferedCode, callbackCode)  \
bufferedCode
#endif

/*
  constants and structures
*/

static const int fixed_scale = 1 << 16;
static const int half_point = 1 << 15;
static const int buffer_size = 2048;

struct LinearGradientValues
{
    qreal dx;
    qreal dy;
    qreal l;
    qreal off;
};

struct RadialGradientValues
{
    qreal dx;
    qreal dy;
    qreal a;
};

struct Operator;
typedef uint *QT_FASTCALL (*DestFetchProc)(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length);
typedef void QT_FASTCALL (*DestStoreProc)(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length);
typedef const uint *QT_FASTCALL (*SourceFetchProc)(uint *buffer, const Operator *o, const QSpanData *data, int y, int x, int length);


struct Operator
{
    QPainter::CompositionMode mode;
    DestFetchProc dest_fetch;
    DestStoreProc dest_store;
    SourceFetchProc src_fetch;
    CompositionFunctionSolid funcSolid;
    CompositionFunction func;
    union {
        LinearGradientValues linear;
        RadialGradientValues radial;
//        TextureValues texture;
    };
};

/*
  Destination fetch. This is simple as we don't have to do bounds checks or
  transformations
*/

static uint * QT_FASTCALL destFetchMono(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
    uchar *data = (uchar *)rasterBuffer->scanLine(y);
    uint *start = buffer;
    const uint *end = buffer + length;
    while (buffer < end) {
        *buffer = data[x>>3] & (0x80 >> (x & 7)) ? 0xff000000 : 0xffffffff;
        ++buffer;
        ++x;
    }
    return start;
}

static uint * QT_FASTCALL destFetchMonoLsb(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
    uchar *data = (uchar *)rasterBuffer->scanLine(y);
    uint *start = buffer;
    const uint *end = buffer + length;
    while (buffer < end) {
        *buffer = data[x>>3] & (0x1 << (x & 7)) ? 0xff000000 : 0xffffffff;
        ++buffer;
        ++x;
    }
    return start;
}

static uint * QT_FASTCALL destFetchRGB32(uint *, QRasterBuffer *rasterBuffer, int x, int y, int)
{
    uint *data = (uint *)rasterBuffer->scanLine(y) + x;
    // This should work without us having to fix the alpha channel manually.
//     for (int i = 0; i < length; ++i)
//         data[i] |= 0xff000000;
    return data;
}

static uint * QT_FASTCALL destFetchARGB32(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
    const uint *data = (const uint *)rasterBuffer->scanLine(y) + x;
    for (int i = 0; i < length; ++i)
        buffer[i] = PREMUL(data[i]);
    return buffer;
}

static uint * QT_FASTCALL destFetchARGB32P(uint *, QRasterBuffer *rasterBuffer, int x, int y, int)
{
    return (uint *)rasterBuffer->scanLine(y) + x;
}

#ifdef Q_WS_QWS
static uint * QT_FASTCALL destFetchRGB16(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
    const ushort *data = (const ushort *)rasterBuffer->scanLine(y) + x;
    for (int i = 0; i < length; ++i)
        buffer[i] = qConvertRgb16To32(data[i]);
    return buffer;
}
#endif

static const DestFetchProc destFetchProc[QImage::NImageFormats] =
{
    0, // Format_Invalid
    destFetchMono, // Format_Mono,
    destFetchMonoLsb, // Format_MonoLSB
    0, // Format_Indexed8
    destFetchRGB32, // Format_RGB32
    destFetchARGB32, // Format_ARGB32,
    destFetchARGB32P // Format_ARGB32_Premultiplied
#ifdef Q_WS_QWS
    ,  destFetchRGB16
#endif
};

/*
  Destination store.
*/


static void QT_FASTCALL destStoreMono(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
    uchar *data = (uchar *)rasterBuffer->scanLine(y);
    for (int i = 0; i < length; ++i) {
        if (qGray(buffer[i]) < int(qt_bayer_matrix[y & 15][x & 15]))
            data[x >> 3] |= 0x80 >> (x & 7);
        else
            data[x >> 3] &= ~(0x80 >> (x & 7));
        ++x;
    }
}

static void QT_FASTCALL destStoreMonoLsb(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
    uchar *data = (uchar *)rasterBuffer->scanLine(y);
    for (int i = 0; i < length; ++i) {
        if (qGray(buffer[i]) < int(qt_bayer_matrix[y & 15][x & 15]))
            data[x >> 3] |= 1 << (x & 7);
        else
            data[x >> 3] &= ~(1 << (x & 7));
        ++x;
    }
}

static void QT_FASTCALL destStoreARGB32(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
    uint *data = (uint *)rasterBuffer->scanLine(y) + x;
    for (int i = 0; i < length; ++i)
        data[i] = INV_PREMUL(buffer[i]);
}

#ifdef Q_WS_QWS
static void QT_FASTCALL destStoreRGB16(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
    ushort *data = (ushort *)rasterBuffer->scanLine(y) + x;
    for (int i = 0; i < length; ++i)
        data[i] = qConvertRgb32To16(buffer[i]);
}
#endif

static const DestStoreProc destStoreProc[QImage::NImageFormats] =
{
    0, // Format_Invalid
    destStoreMono, // Format_Mono,
    destStoreMonoLsb, // Format_MonoLSB
    0, // Format_Indexed8
    0, // Format_RGB32
    destStoreARGB32, // Format_ARGB32,
    0 // Format_ARGB32_Premultiplied
#ifdef Q_WS_QWS
    ,  destStoreRGB16
#endif
};

/*
  Source fetches

  This is a bit more complicated, as we need several fetch routines for every surface type

  We need 5 fetch methods per surface type:
  untransformed
  transformed
  transformed tiled
  transformed bilinear
  transformed bilinear tiled

  We don't need bounds checks for untransformed, but we need them for the other ones.

  The generic implementation does pixel by pixel fetches
*/

static uint QT_FASTCALL fetchPixel_Mono(const uchar *scanLine, int x, const QVector<QRgb> *rgb)
{
    bool pixel = scanLine[x>>3] & (0x80 >> (x & 7));
    if (rgb) return rgb->at(pixel ? 1 : 0);
    return pixel ? 0xff000000 : 0xffffffff;
}

static uint QT_FASTCALL fetchPixel_MonoLSB(const uchar *scanLine, int x, const QVector<QRgb> *rgb)
{
    bool pixel = scanLine[x>>3] & (0x1 << (x & 7));
    if (rgb) return rgb->at(pixel ? 1 : 0);
    return pixel ? 0xff000000 : 0xffffffff;
}

static uint QT_FASTCALL fetchPixel_Indexed8(const uchar *scanLine, int x, const QVector<QRgb> *rgb)
{
    return PREMUL(rgb->at(scanLine[x]));
}

static uint QT_FASTCALL fetchPixel_RGB32(const uchar *scanLine, int x, const QVector<QRgb> *)
{
    return ((const uint *)scanLine)[x] | 0xff000000;
}

static uint QT_FASTCALL fetchPixel_ARGB32(const uchar *scanLine, int x, const QVector<QRgb> *)
{
    return PREMUL(((const uint *)scanLine)[x]);
}

static uint QT_FASTCALL fetchPixel_ARGB32_Premultiplied(const uchar *scanLine, int x, const QVector<QRgb> *)
{
    return ((const uint *)scanLine)[x];
}

#ifdef Q_WS_QWS
static uint QT_FASTCALL fetchPixel_RGB16(const uchar *scanLine, int x, const QVector<QRgb> *)
{
    return qConvertRgb16To32(((const ushort *)scanLine)[x]);
}
#endif

typedef uint QT_FASTCALL (*FetchPixelProc)(const uchar *scanLine, int x, const QVector<QRgb> *);

static const FetchPixelProc fetchPixelProc[QImage::NImageFormats] =
{
    0,
    fetchPixel_Mono,
    fetchPixel_MonoLSB,
    fetchPixel_Indexed8,
    fetchPixel_RGB32,
    fetchPixel_ARGB32,
    fetchPixel_ARGB32_Premultiplied
#ifdef Q_WS_QWS
    ,  fetchPixel_RGB16
#endif
};

enum TextureBlendType {
    BlendUntransformed,
    BlendTiled,
    BlendTransformed,
    BlendTransformedTiled,
    BlendTransformedBilinear,
    BlendTransformedBilinearTiled,
    NBlendTypes
};

static const uint * QT_FASTCALL fetch_generic(uint *buffer, const Operator *, const QSpanData *data,
                                             int y, int x, int length)
{
    FetchPixelProc fetch = fetchPixelProc[data->texture.format];
    const uchar *scanLine = data->texture.scanLine(y);
    for (int i = 0; i < length; ++i)
        buffer[i] = fetch(scanLine, x + i, data->texture.colorTable);
    return buffer;
}

static const uint * QT_FASTCALL fetchTransformed_generic(uint *buffer, const Operator *, const QSpanData *data,
                                                         int y, int x, int length)
{
    FetchPixelProc fetch = fetchPixelProc[data->texture.format];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    int fx = int((data->m21 * (y + 0.5)
                  + data->m11 * (x + 0.5) + data->dx) * fixed_scale);
    int fy = int((data->m22 * (y + 0.5)
                  + data->m12 * (x + 0.5) + data->dy) * fixed_scale);

    const uint *end = buffer + length;
    uint *b = buffer;
    while (b < end) {
        int px = fx >> 16;
        int py = fy >> 16;

        bool out = (px < 0) || (px >= image_width)
                   || (py < 0) || (py >= image_height);

        const uchar *scanLine = data->texture.scanLine(py);
        *b = out ? uint(0) : fetch(scanLine, px, data->texture.colorTable);
        fx += fdx;
        fy += fdy;
        ++b;
    }
    return buffer;
}

static const uint * QT_FASTCALL fetchTransformedTiled_generic(uint *buffer, const Operator *, const QSpanData *data,
                                                              int y, int x, int length)
{
    FetchPixelProc fetch = fetchPixelProc[data->texture.format];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    int fx = int((data->m21 * (y + 0.5)
                  + data->m11 * (x + 0.5) + data->dx) * fixed_scale);
    int fy = int((data->m22 * (y + 0.5)
                  + data->m12 * (x + 0.5) + data->dy) * fixed_scale);

    const uint *end = buffer + length;
    uint *b = buffer;
    while (b < end) {
        int px = fx >> 16;
        int py = fy >> 16;

        px %= image_width;
        py %= image_height;
        if (px < 0) px += image_width;
        if (py < 0) py += image_height;

        const uchar *scanLine = data->texture.scanLine(py);
        *b = fetch(scanLine, px, data->texture.colorTable);
        fx += fdx;
        fy += fdy;
        ++b;
    }
    return buffer;
}

static const uint * QT_FASTCALL fetchTransformedBilinear_generic(uint *buffer, const Operator *, const QSpanData *data,
                                                                 int y, int x, int length)
{
    FetchPixelProc fetch = fetchPixelProc[data->texture.format];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    int fx = int((data->m21 * (y + 0.5)
                  + data->m11 * (x + 0.5) + data->dx) * fixed_scale);
    int fy = int((data->m22 * (y + 0.5)
                  + data->m12 * (x + 0.5) + data->dy) * fixed_scale);

    const uint *end = buffer + length;
    uint *b = buffer;
    while (b < end) {
        int x1 = (fx >> 16);
        int x2 = x1 + 1;
        int y1 = (fy >> 16);
        int y2 = y1 + 1;

        int distx = ((fx - (x1 << 16)) >> 8);
        int disty = ((fy - (y1 << 16)) >> 8);
        int idistx = 256 - distx;
        int idisty = 256 - disty;

        bool x1_out = ((x1 < 0) || (x1 >= image_width));
        bool x2_out = ((x2 < 0) || (x2 >= image_width));
        bool y1_out = ((y1 < 0) || (y1 >= image_height));
        bool y2_out = ((y2 < 0) || (y2 >= image_height));

        const uchar *s1 = data->texture.scanLine(y1);
        const uchar *s2 = s1 + data->texture.bytesPerLine;

        uint tl = (x1_out || y1_out) ? uint(0) : fetch(s1, x1, data->texture.colorTable);
        uint tr = (x2_out || y1_out) ? uint(0) : fetch(s1, x2, data->texture.colorTable);
        uint bl = (x1_out || y2_out) ? uint(0) : fetch(s2, x1, data->texture.colorTable);
        uint br = (x2_out || y2_out) ? uint(0) : fetch(s2, x2, data->texture.colorTable);

        uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
        uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
        *b = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);

        fx += fdx;
        fy += fdy;
        ++b;
    }
    return buffer;
}

static const uint * QT_FASTCALL fetchTransformedBilinearTiled_generic(uint *buffer, const Operator *, const QSpanData *data,
                                                                     int y, int x, int length)
{
    FetchPixelProc fetch = fetchPixelProc[data->texture.format];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    int fx = int((data->m21 * (y + 0.5)
                  + data->m11 * (x + 0.5) + data->dx) * fixed_scale);
    int fy = int((data->m22 * (y + 0.5)
                  + data->m12 * (x + 0.5) + data->dy) * fixed_scale);

    const uint *end = buffer + length;
    uint *b = buffer;
    while (b < end) {
        int x1 = (fx >> 16);
        int x2 = x1 + 1;
        int y1 = (fy >> 16);
        int y2 = y1 + 1;

        int distx = ((fx - (x1 << 16)) >> 8);
        int disty = ((fy - (y1 << 16)) >> 8);
        int idistx = 256 - distx;
        int idisty = 256 - disty;

        x1 %= image_width;
        x2 %= image_width;
        y1 %= image_height;
        y2 %= image_height;

        if (x1 < 0) x1 += image_width;
        if (x2 < 0) x2 += image_width;
        if (y1 < 0) y1 += image_height;
        if (y2 < 0) y2 += image_height;

        Q_ASSERT(x1 >= 0 && x1 < image_width);
        Q_ASSERT(x2 >= 0 && x2 < image_width);
        Q_ASSERT(y1 >= 0 && y1 < image_height);
        Q_ASSERT(y2 >= 0 && y2 < image_height);

        const uchar *s1 = data->texture.scanLine(y1);
        const uchar *s2 = data->texture.scanLine(y2);

        uint tl = fetch(s1, x1, data->texture.colorTable);
        uint tr = fetch(s1, x2, data->texture.colorTable);
        uint bl = fetch(s2, x1, data->texture.colorTable);
        uint br = fetch(s2, x2, data->texture.colorTable);

        uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
        uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
        *b = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);

        fx += fdx;
        fy += fdy;
        ++b;
    }
    return buffer;
}


static const SourceFetchProc sourceFetch[NBlendTypes][QImage::NImageFormats] = {
    // Untransformed
    {
        0, // Invalid
        fetch_generic,   // Mono
        fetch_generic,   // MonoLsb
        fetch_generic,   // Indexed8
        fetch_generic,   // RGB32
        fetch_generic,   // ARGB32
        fetch_generic
#ifdef Q_WS_QWS
        ,  fetch_generic
#endif
    }, // ARGB32_Premultiplied
    // Tiled
    {
        0, // Invalid
        fetch_generic,   // Mono
        fetch_generic,   // MonoLsb
        fetch_generic,   // Indexed8
        fetch_generic,   // RGB32
        fetch_generic,   // ARGB32
        fetch_generic
#ifdef Q_WS_QWS
        ,  fetch_generic
#endif
    }, // ARGB32_Premultiplied
    // Transformed
    {
        0, // Invalid
        fetchTransformed_generic,   // Mono
        fetchTransformed_generic,   // MonoLsb
        fetchTransformed_generic,   // Indexed8
        fetchTransformed_generic,   // RGB32
        fetchTransformed_generic,   // ARGB32
        fetchTransformed_generic
#ifdef Q_WS_QWS
        ,  fetchTransformed_generic
#endif
    }, // ARGB32_Premultiplied
    {
        0, // TransformedTiled
        fetchTransformedTiled_generic,   // Mono
        fetchTransformedTiled_generic,   // MonoLsb
        fetchTransformedTiled_generic,   // Indexed8
        fetchTransformedTiled_generic,   // RGB32
        fetchTransformedTiled_generic,   // ARGB32
        fetchTransformedTiled_generic
#ifdef Q_WS_QWS
        ,  fetchTransformedTiled_generic
#endif
    }, // ARGB32_Premultiplied
    {
        0, // Bilinear
        fetchTransformedBilinear_generic,   // Mono
        fetchTransformedBilinear_generic,   // MonoLsb
        fetchTransformedBilinear_generic,   // Indexed8
        fetchTransformedBilinear_generic,   // RGB32
        fetchTransformedBilinear_generic,   // ARGB32
        fetchTransformedBilinear_generic
#ifdef Q_WS_QWS
        ,  fetchTransformedBilinear_generic
#endif
    }, // ARGB32_Premultiplied
    {
        0, // BilinearTiled
        fetchTransformedBilinearTiled_generic,   // Mono
        fetchTransformedBilinearTiled_generic,   // MonoLsb
        fetchTransformedBilinearTiled_generic,   // Indexed8
        fetchTransformedBilinearTiled_generic,   // RGB32
        fetchTransformedBilinearTiled_generic,   // ARGB32
        fetchTransformedBilinearTiled_generic
#ifdef Q_WS_QWS
        ,  fetchTransformedBilinearTiled_generic
#endif
    }, // ARGB32_Premultiplied
};


static uint qt_gradient_pixel(const GradientData *data, qreal pos)
{
    int ipos = qRound(pos * GRADIENT_STOPTABLE_SIZE - 1);

  // calculate the actual offset.
    if (ipos < 0 || ipos >= GRADIENT_STOPTABLE_SIZE) {
        if (data->spread == QGradient::RepeatSpread) {
            ipos = ipos % GRADIENT_STOPTABLE_SIZE;
            ipos = ipos < 0 ? GRADIENT_STOPTABLE_SIZE + ipos : ipos;

        } else if (data->spread == QGradient::ReflectSpread) {
            const int limit = GRADIENT_STOPTABLE_SIZE * 2 - 1;
            ipos = ipos % limit;
            ipos = ipos < 0 ? limit + ipos : ipos;
            ipos = ipos >= GRADIENT_STOPTABLE_SIZE ? limit - ipos : ipos;

        } else {
            if (ipos < 0) ipos = 0;
            else if (ipos >= GRADIENT_STOPTABLE_SIZE) ipos = GRADIENT_STOPTABLE_SIZE-1;
        }
    }

    Q_ASSERT(ipos >= 0);
    Q_ASSERT(ipos < GRADIENT_STOPTABLE_SIZE);

    return data->colorTable[ipos];
}

#ifdef Q_WS_QWS

#define FIXPT_BITS 8
#define FIXPT_SIZE (1<<FIXPT_BITS)

static uint qt_gradient_pixel_fixed(const GradientData *data, int fixed_pos)
{
    int ipos = ((fixed_pos + FIXPT_SIZE / 2) >> FIXPT_BITS) - 1;

    // calculate the actual offset.
    if (ipos < 0 || ipos >= GRADIENT_STOPTABLE_SIZE) {
        if (data->spread == QGradient::RepeatSpread) {
            ipos = ipos % GRADIENT_STOPTABLE_SIZE;
            ipos = ipos < 0 ? GRADIENT_STOPTABLE_SIZE + ipos : ipos;

        } else if (data->spread == QGradient::ReflectSpread) {
            const int limit = GRADIENT_STOPTABLE_SIZE * 2 - 1;
            ipos = ipos % limit;
            ipos = ipos < 0 ? limit + ipos : ipos;
            ipos = ipos >= GRADIENT_STOPTABLE_SIZE ? limit - ipos : ipos;

        } else {
            if (ipos < 0) ipos = 0;
            else if (ipos >= GRADIENT_STOPTABLE_SIZE) ipos = GRADIENT_STOPTABLE_SIZE-1;
        }
    }

    Q_ASSERT(ipos >= 0);
    Q_ASSERT(ipos < GRADIENT_STOPTABLE_SIZE);

    return data->colorTable[ipos];
}
#endif

static void QT_FASTCALL getLinearGradientValues(LinearGradientValues *v, const QSpanData *data)
{
    v->dx = data->gradient.linear.end.x - data->gradient.linear.origin.x;
    v->dy = data->gradient.linear.end.y - data->gradient.linear.origin.y;
    v->l = v->dx * v->dx + v->dy * v->dy;
    v->off = 0;
    if (v->l != 0) {
        v->dx /= v->l;
        v->dy /= v->l;
        v->off = -v->dx * data->gradient.linear.origin.x - v->dy * data->gradient.linear.origin.y;
    }
}

#ifdef Q_WS_QWS
static const uint * QT_FASTCALL fetchLinearGradient(uint *buffer, const Operator *op, const QSpanData *data,
                                                    int y, int x, int length)
{
    const uint *b = buffer;
    qreal t, inc;
    if (op->linear.l == 0) {
        t = inc = 0;
    } else {
        qreal rx = data->m21 * y + data->m11 * x + data->dx;
        qreal ry = data->m22 * y + data->m12 * x + data->dy;
        t = op->linear.dx*rx + op->linear.dy*ry + op->linear.off;
        inc = op->linear.dx * data->m11 + op->linear.dy * data->m12;
        t *= GRADIENT_STOPTABLE_SIZE;
        inc *= GRADIENT_STOPTABLE_SIZE;
    }

    const uint *end = buffer + length;
    if (inc > -1e-5 && inc < 1e-5) {
        QT_MEMFILL_UINT(buffer, length, qt_gradient_pixel_fixed(&data->gradient, int(t * FIXPT_SIZE)));
    } else {
        if (t+inc*length < qreal(INT_MAX >> (FIXPT_BITS + 1)) &&
            t+inc*length > qreal(INT_MIN >> (FIXPT_BITS + 1))) {
            // we can use fixed point math
            int t_fixed = int(t * FIXPT_SIZE);
            int inc_fixed = int(inc * FIXPT_SIZE);
            while (buffer < end) {
                *buffer = qt_gradient_pixel_fixed(&data->gradient, t_fixed);
                t_fixed += inc_fixed;
                ++buffer;
            }
        } else {
            // we have to fall back to float math
            while (buffer < end) {
                *buffer = qt_gradient_pixel(&data->gradient, t/GRADIENT_STOPTABLE_SIZE);
                t += inc;
                ++buffer;
            }
        }
    }
    return b;
}
#else
static const uint * QT_FASTCALL fetchLinearGradient(uint *buffer, const Operator *op, const QSpanData *data,
                                                    int y, int x, int length)
{
    const uint *b = buffer;
    qreal t, inc;
    if (op->linear.l == 0) {
        t = inc = 0;
    } else {
        qreal rx = data->m21 * y + data->m11 * x + data->dx;
        qreal ry = data->m22 * y + data->m12 * x + data->dy;
        t = op->linear.dx*rx + op->linear.dy*ry + op->linear.off;
        inc = op->linear.dx * data->m11 + op->linear.dy * data->m12;
    }

    const uint *end = buffer + length;
    if (inc > -1e-6 && inc < 1e-6) {
        QT_MEMFILL_UINT(buffer, length, qt_gradient_pixel(&data->gradient, t));
    } else {
        while (buffer < end) {
            *buffer = qt_gradient_pixel(&data->gradient, t);
            t += inc;
            ++buffer;
        }
    }
    return b;
}
#endif

static inline qreal determinant(qreal a, qreal b, qreal c)
{
    return (b * b) - (4 * a * c);
}

// function to evaluate real roots
static inline qreal realRoots(qreal a, qreal b, qreal detSqrt)
{
    return (-b + detSqrt)/(2 * a);
}

static void QT_FASTCALL getRadialGradientValues(RadialGradientValues *v, const QSpanData *data)
{
    v->dx = data->gradient.radial.center.x - data->gradient.radial.focal.x;
    v->dy = data->gradient.radial.center.y - data->gradient.radial.focal.y;
    v->a = data->gradient.radial.radius*data->gradient.radial.radius - v->dx*v->dx - v->dy*v->dy;
}

static const uint * QT_FASTCALL fetchRadialGradient(uint *buffer, const Operator *op, const QSpanData *data,
                                                    int y, int x, int length)
{
    const uint *b = buffer;
    qreal rx = data->m21 * (y + 0.5)
               + data->dx + data->m11 * (x + 0.5) - data->gradient.radial.focal.x;
    qreal ry = data->m22 * (y + 0.5)
               + data->dy + data->m12 * (x + 0.5) - data->gradient.radial.focal.y;
    //qreal r  = data->gradient.radial.radius;

    const uint *end = buffer + length;
    while (buffer < end) {
        qreal b  = 2*(rx*op->radial.dx + ry*op->radial.dy);
        qreal det = determinant(op->radial.a, b , -(rx*rx + ry*ry));
        qreal s = realRoots(op->radial.a, b, sqrt(det));

        *buffer = qt_gradient_pixel(&data->gradient, s);

        rx += data->m11;
        ry += data->m12;
        ++buffer;
    }
    return b;
}

static const uint * QT_FASTCALL fetchConicalGradient(uint *buffer, const Operator *, const QSpanData *data,
                                                     int y, int x, int length)
{
    const uint *b = buffer;
    qreal rx = data->m21 * (y + 0.5)
               + data->dx + data->m11 * (x + 0.5) - data->gradient.conical.center.x;
    qreal ry = data->m22 * (y + 0.5)
               + data->dy + data->m12 * (x + 0.5) - data->gradient.conical.center.y;

    const uint *end = buffer + length;
    while (buffer < end) {
        qreal angle = atan2(ry, rx) + data->gradient.conical.angle;

        *buffer = qt_gradient_pixel(&data->gradient, 1. - angle / (2*Q_PI));

        rx += data->m11;
        ry += data->m12;
        ++buffer;
    }
    return b;
}



/* The constant alpha factor describes an alpha factor that gets applied
   to the result of the composition operation combining it with the destination.

   The intent is that if const_alpha == 0. we get back dest, and if const_alpha == 1.
   we get the unmodified operation

   result = src op dest
   dest = result * const_alpha + dest * (1. - const_alpha)

   This means that in the comments below, the first line is the const_alpha==255 case, the
   second line the general one.

   In the lines below:
   s == src, sa == alpha(src), sia = 1 - alpha(src)
   d == dest, da == alpha(dest), dia = 1 - alpha(dest)
   ca = const_alpha, cia = 1 - const_alpha

   The methods exist in two variants. One where we have a constant source, the other
   where the source is an array of pixels.
*/

/*
  result = 0
  d = d * cia
*/
static void QT_FASTCALL comp_func_solid_Clear(uint *dest, int length, uint, uint const_alpha)
{
    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, 0);
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], ialpha);
    }
}

static void QT_FASTCALL comp_func_Clear(uint *dest, const uint *, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, 0);
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], ialpha);
    }
}

/*
  result = s
  dest = s * ca + d * cia
*/
static void QT_FASTCALL comp_func_solid_Source(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, color);
    } else {
        int ialpha = 255 - const_alpha;
        color = BYTE_MUL(color, const_alpha);
        for (int i = 0; i < length; ++i)
            dest[i] = color + BYTE_MUL(dest[i], ialpha);
    }
}

static void QT_FASTCALL comp_func_Source(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        ::memcpy(dest, src, length * sizeof(uint));
    } else {
        int ialpha = 255 - const_alpha;
        for (int i = 0; i < length; ++i)
            dest[i] = INTERPOLATE_PIXEL_255(src[i], const_alpha, dest[i], ialpha);
    }
}

/*
  result = s + d * sia
  dest = (s + d * sia) * ca + d * cia
       = s * ca + d * (sia * ca + cia)
       = s * ca + d * (1 - sa*ca)
*/
static void QT_FASTCALL comp_func_solid_SourceOver(uint *dest, int length, uint color, uint const_alpha)
{
    if ((const_alpha & qAlpha(color)) == 255) {
        QT_MEMFILL_UINT(dest, length, color);
    } else {
        if (const_alpha != 255)
            color = BYTE_MUL(color, const_alpha);
        for (int i = 0; i < length; ++i)
            dest[i] = color + BYTE_MUL(dest[i], qAlpha(~color));
    }
}

static void QT_FASTCALL comp_func_SourceOver(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            uint s = src[i];
            dest[i] = s + BYTE_MUL(dest[i], qAlpha(~s));
        }
    } else {
        for (int i = 0; i < length; ++i) {
            uint s = BYTE_MUL(src[i], const_alpha);
            dest[i] = s + BYTE_MUL(dest[i], qAlpha(~s));
        }
    }
}

/*
  result = d + s * dia
  dest = (d + s * dia) * ca + d * cia
       = d + s * dia * ca
*/
static void QT_FASTCALL comp_func_solid_DestinationOver(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha != 255)
        color = BYTE_MUL(color, const_alpha);
    for (int i = 0; i < length; ++i) {
        uint d = dest[i];
        dest[i] = d + BYTE_MUL(color, qAlpha(~d));
    }
}

static void QT_FASTCALL comp_func_DestinationOver(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            uint d = dest[i];
            dest[i] = d + BYTE_MUL(src[i], qAlpha(~d));
        }
    } else {
        for (int i = 0; i < length; ++i) {
            uint d = dest[i];
            uint s = BYTE_MUL(src[i], const_alpha);
            dest[i] = d + BYTE_MUL(s, qAlpha(~d));
        }
    }
}

/*
  result = s * da
  dest = s * da * ca + d * cia
*/
static void QT_FASTCALL comp_func_solid_SourceIn(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(color, qAlpha(dest[i]));
    } else {
        color = BYTE_MUL(color, const_alpha);
        uint cia = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(color, qAlpha(d), d, cia);
        }
    }
}

static void QT_FASTCALL comp_func_SourceIn(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(src[i], qAlpha(dest[i]));
    } else {
        uint cia = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint d = dest[i];
            uint s = BYTE_MUL(src[i], const_alpha);
            dest[i] = INTERPOLATE_PIXEL_255(s, qAlpha(d), d, cia);
        }
    }
}

/*
  result = d * sa
  dest = d * sa * ca + d * cia
       = d * (sa * ca + cia)
*/
static void QT_FASTCALL comp_func_solid_DestinationIn(uint *dest, int length, uint color, uint const_alpha)
{
    uint a = qAlpha(color);
    if (const_alpha != 255) {
        a = BYTE_MUL(a, const_alpha) + 255 - const_alpha;
    }
    for (int i = 0; i < length; ++i) {
        dest[i] = BYTE_MUL(dest[i], a);
    }
}

static void QT_FASTCALL comp_func_DestinationIn(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], qAlpha(src[i]));
    } else {
        int cia = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint a = BYTE_MUL(qAlpha(src[i]), const_alpha) + cia;
            dest[i] = BYTE_MUL(dest[i], a);
        }
    }
}

/*
  result = s * dia
  dest = s * dia * ca + d * cia
*/

static void QT_FASTCALL comp_func_solid_SourceOut(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(color, qAlpha(~dest[i]));
    } else {
        color = BYTE_MUL(color, const_alpha);
        int cia = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(color, qAlpha(~d), d, cia);
        }
    }
}

static void QT_FASTCALL comp_func_SourceOut(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(src[i], qAlpha(~dest[i]));
    } else {
        int cia = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint s = BYTE_MUL(src[i], const_alpha);
            uint d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, qAlpha(~d), d, cia);
        }
    }
}

/*
  result = d * sia
  dest = d * sia * ca + d * cia
       = d * (sia * ca + cia)
*/
static void QT_FASTCALL comp_func_solid_DestinationOut(uint *dest, int length, uint color, uint const_alpha)
{
    uint a = qAlpha(~color);
    if (const_alpha != 255)
        a = BYTE_MUL(a, const_alpha) + 255 - const_alpha;
    for (int i = 0; i < length; ++i)
        dest[i] = BYTE_MUL(dest[i], a);
}

static void QT_FASTCALL comp_func_DestinationOut(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i)
            dest[i] = BYTE_MUL(dest[i], qAlpha(~src[i]));
    } else {
        int cia = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint sia = BYTE_MUL(qAlpha(~src[i]), const_alpha) + cia;
            dest[i] = BYTE_MUL(dest[i], sia);
        }
    }
}

/*
  result = s*da + d*sia
  dest = s*da*ca + d*sia*ca + d *cia
       = s*ca * da + d * (sia*ca + cia)
       = s*ca * da + d * (1 - sa*ca)
*/
static void QT_FASTCALL comp_func_solid_SourceAtop(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha != 255) {
        color = BYTE_MUL(color, const_alpha);
    }
    uint sia = qAlpha(~color);
    for (int i = 0; i < length; ++i)
        dest[i] = INTERPOLATE_PIXEL_255(color, qAlpha(dest[i]), dest[i], sia);
}

static void QT_FASTCALL comp_func_SourceAtop(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            uint s = src[i];
            uint d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, qAlpha(d), d, qAlpha(~s));
        }
    } else {
        for (int i = 0; i < length; ++i) {
            uint s = BYTE_MUL(src[i], const_alpha);
            uint d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, qAlpha(d), d, qAlpha(~s));
        }
    }
}

/*
  result = d*sa + s*dia
  dest = d*sa*ca + s*dia*ca + d *cia
       = s*ca * dia + d * (sa*ca + cia)
*/
static void QT_FASTCALL comp_func_solid_DestinationAtop(uint *dest, int length, uint color, uint const_alpha)
{
    uint a = qAlpha(color);
    if (const_alpha != 255) {
        color = BYTE_MUL(color, const_alpha);
        a = qAlpha(color) + 255 - const_alpha;
    }
    for (int i = 0; i < length; ++i) {
        uint d = dest[i];
        dest[i] = INTERPOLATE_PIXEL_255(d, a, color, qAlpha(~d));
    }
}

static void QT_FASTCALL comp_func_DestinationAtop(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            uint s = src[i];
            uint d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(d, qAlpha(s), s, qAlpha(~d));
        }
    } else {
        int cia = 255 - const_alpha;
        for (int i = 0; i < length; ++i) {
            uint s = BYTE_MUL(src[i], const_alpha);
            uint d = dest[i];
            uint a = qAlpha(s) + cia;
            dest[i] = INTERPOLATE_PIXEL_255(d, a, s, qAlpha(~d));
        }
    }
}

/*
  result = d*sia + s*dia
  dest = d*sia*ca + s*dia*ca + d *cia
       = s*ca * dia + d * (sia*ca + cia)
       = s*ca * dia + d * (1 - sa*ca)
*/
static void QT_FASTCALL comp_func_solid_XOR(uint *dest, int length, uint color, uint const_alpha)
{
    if (const_alpha != 255)
        color = BYTE_MUL(color, const_alpha);
    uint sia = qAlpha(~color);

    for (int i = 0; i < length; ++i) {
        uint d = dest[i];
        dest[i] = INTERPOLATE_PIXEL_255(color, qAlpha(~d), d, sia);
    }
}

static void QT_FASTCALL comp_func_XOR(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            uint d = dest[i];
            uint s = src[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, qAlpha(~d), d, qAlpha(~s));
        }
    } else {
        for (int i = 0; i < length; ++i) {
            uint d = dest[i];
            uint s = BYTE_MUL(src[i], const_alpha);
            dest[i] = INTERPOLATE_PIXEL_255(s, qAlpha(~d), d, qAlpha(~s));
        }
    }
}

static const CompositionFunctionSolid functionForModeSolid_C[] = {
        comp_func_solid_SourceOver,
        comp_func_solid_DestinationOver,
        comp_func_solid_Clear,
        comp_func_solid_Source,
        0,
        comp_func_solid_SourceIn,
        comp_func_solid_DestinationIn,
        comp_func_solid_SourceOut,
        comp_func_solid_DestinationOut,
        comp_func_solid_SourceAtop,
        comp_func_solid_DestinationAtop,
        comp_func_solid_XOR
};

static const CompositionFunctionSolid *functionForModeSolid = functionForModeSolid_C;

static const CompositionFunction functionForMode_C[] = {
        comp_func_SourceOver,
        comp_func_DestinationOver,
        comp_func_Clear,
        comp_func_Source,
        0,
        comp_func_SourceIn,
        comp_func_DestinationIn,
        comp_func_SourceOut,
        comp_func_DestinationOut,
        comp_func_SourceAtop,
        comp_func_DestinationAtop,
        comp_func_XOR
};

static const CompositionFunction *functionForMode = functionForMode_C;

static TextureBlendType getBlendType(const QSpanData *data)
{
    TextureBlendType ft;
    if (data->txop <= QPainterPrivate::TxTranslate)
        if (data->texture.type == TextureData::Tiled)
            ft = BlendTiled;
        else
            ft = BlendUntransformed;
    else if (data->bilinear)
        if (data->texture.type == TextureData::Tiled)
            ft = BlendTransformedBilinearTiled;
        else
            ft = BlendTransformedBilinear;
    else
        if (data->texture.type == TextureData::Tiled)
            ft = BlendTransformedTiled;
        else
            ft = BlendTransformed;
    return ft;
}

static inline Operator getOperator(const QSpanData *data, const QSpan *spans, int spanCount)
{
    Operator op;
    bool solidSource = false;

    switch(data->type) {
    case QSpanData::Solid:
        solidSource = (qAlpha(data->solid.color) == 255);
        break;
    case QSpanData::LinearGradient:
        solidSource = !data->gradient.alphaColor;
        getLinearGradientValues(&op.linear, data);
        op.src_fetch = fetchLinearGradient;
        break;
    case QSpanData::RadialGradient:
        solidSource = !data->gradient.alphaColor;
        getRadialGradientValues(&op.radial, data);
        op.src_fetch = fetchRadialGradient;
        break;
    case QSpanData::ConicalGradient:
        solidSource = !data->gradient.alphaColor;
        op.src_fetch = fetchConicalGradient;
        break;
    case QSpanData::Texture:
        op.src_fetch = sourceFetch[getBlendType(data)][data->texture.format];
        solidSource = data->texture.format != QImage::Format_ARGB32_Premultiplied
                      && data->texture.format != QImage::Format_ARGB32
                      && data->texture.format != QImage::Format_Indexed8;
    default:
        break;
    }

    op.mode = data->rasterBuffer->compositionMode;
    if (op.mode == QPainter::CompositionMode_SourceOver && solidSource)
        op.mode = QPainter::CompositionMode_Source;

    op.dest_fetch = destFetchProc[data->rasterBuffer->format];
    if (op.mode == QPainter::CompositionMode_Source) {
        switch (data->rasterBuffer->format) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
            // this one sets up the pointer correctly so we save one copy
            op.dest_fetch = destFetchProc[QImage::Format_ARGB32_Premultiplied];
            break;
        default: {
            const QSpan *lastSpan = spans + spanCount;
            bool alphaSpans = false;
            while (spans < lastSpan) {
                if (spans->coverage != 255) {
                    alphaSpans = true;
                    break;
                }
                ++spans;
            }
            if (!alphaSpans)
                op.dest_fetch = 0;
        }
        }
    }

    op.dest_store = destStoreProc[data->rasterBuffer->format];

    op.funcSolid = functionForModeSolid[op.mode];
    op.func = functionForMode[op.mode];

    return op;
}



// -------------------- blend methods ---------------------


static void blend_color_generic(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    QWS_CALLBACK_IF_NONBUFFERED(data, {}, {
        data->rasterEngine->drawColorSpans(spans, count, data->solid.color);
        return;
    })

    uint buffer[buffer_size];
    Operator op = getOperator(data, spans, count);
    if (!op.funcSolid)
        return;

    while (count--) {
        int x = spans->x;
        int length = spans->len;
        while (length) {
            int l = qMin(buffer_size, length);
            uint *dest = op.dest_fetch ? op.dest_fetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
            op.funcSolid(dest, l, data->solid.color, spans->coverage);
            if (op.dest_store)
                op.dest_store(data->rasterBuffer, x, spans->y, dest, l);
            length -= l;
            x += l;
        }
        ++spans;
    }
}

static void blend_color_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    QWS_CALLBACK_IF_NONBUFFERED(data, {}, {
        data->rasterEngine->drawColorSpans(spans, count, data->solid.color);
        return;
    })

    Operator op = getOperator(data, spans, count);
    if (!op.funcSolid)
        return;

    if (op.mode == QPainter::CompositionMode_Source) {
        // inline for performance
        while (count--) {
            uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
            if (spans->coverage == 255) {
                QT_MEMFILL_UINT(target, spans->len, data->solid.color);
            } else {
                uint c = BYTE_MUL(data->solid.color, spans->coverage);
                int ialpha = 255 - spans->coverage;
                for (int i = 0; i < spans->len; ++i)
                    target[i] = c + BYTE_MUL(target[i], ialpha);
            }
            ++spans;
        }
        return;
    }

    while (count--) {
        uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
        op.funcSolid(target, spans->len, data->solid.color, spans->coverage);
        ++spans;
    }
}

#ifdef Q_WS_QWS

static inline uint BYTE_MUL_RGB16(uint x, uint a) {
    a += 1;
    uint t = (((x & 0x07e0)*a) >> 8) & 0x07e0;
    t |= (((x & 0xf81f)*(a>>2)) >> 6) & 0xf81f;
    return t;
}

static void blend_color_rgb16(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    Operator op = getOperator(data, spans, count);
    if (!op.funcSolid)
        return;

    if (op.mode == QPainter::CompositionMode_Source) {
        // inline for performance
        ushort c = qConvertRgb32To16(data->solid.color);
        while (count--) {
            ushort *target = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
            if (spans->coverage == 255) {
                QT_MEMFILL_USHORT(target, spans->len, c);
            } else {
                ushort color = BYTE_MUL_RGB16(c, spans->coverage);
                int ialpha = 255 - spans->coverage;
                const ushort *end = target + spans->len;
                while (target < end) {
                    *target = color + BYTE_MUL_RGB16(*target, ialpha);
                    ++target;
                }
            }
            ++spans;
        }
        return;
    }

    Q_ASSERT(op.mode == QPainter::CompositionMode_SourceOver);

    while (count--) {
        uint color = BYTE_MUL(data->solid.color, spans->coverage);
        int ialpha = qAlpha(~color);
        ushort c = qConvertRgb32To16(color);
        ushort *target = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
        const ushort *end = target + spans->len;
        while (target < end) {
            *target = c + BYTE_MUL_RGB16(*target, ialpha);
            ++target;
        }
        ++spans;
    }
}
#endif


static void blend_src_generic(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    uint buffer[buffer_size];
    uint src_buffer[buffer_size];
    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    while (count--) {
        int x = spans->x;
        int length = spans->len;
        while (length) {
            int l = qMin(buffer_size, length);
            const uint *src = op.src_fetch(src_buffer, &op, data, spans->y, x, l);
            QWS_CALLBACK_IF_NONBUFFERED(data,
            {
                uint *dest = op.dest_fetch ? op.dest_fetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
                op.func(dest, src, l, spans->coverage);
                if (op.dest_store)
                    op.dest_store(data->rasterBuffer, x, spans->y, dest, l);
            }, {
                data->rasterEngine->drawBufferSpan(src, l, x, spans->y, l,
                                                   spans->coverage);
            })
            x += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_src_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    uint src_buffer[buffer_size];
    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    while (count--) {
        uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y));
        int x = spans->x;
        int length = spans->len;
        while (length) {
            int l = qMin(length, buffer_size);
            const uint *src = op.src_fetch(src_buffer, &op, data, spans->y, x, l);
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                    op.func(target + x, src, l, spans->coverage);
            }, {
                data->rasterEngine->drawBufferSpan(src, buffer_size,
                                                   x, spans->y, l,
                                                   spans->coverage);
            })
            x += l;
            length -= l;
        }
        ++spans;
    }
}




static void blend_untransformed_generic(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    uint buffer[buffer_size];
    uint src_buffer[buffer_size];
    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    const int image_width = data->texture.width;
    const int image_height = data->texture.height;
    int xoff = qRound(data->dx);
    int yoff = qRound(data->dy);

    while (count--) {
        int x = spans->x;
        int length = spans->len;
        int sx = xoff + x;
        int sy = yoff + spans->y;
        if (sy >= 0 && sy < image_height && sx < image_width) {
            if (sx < 0) {
                x -= sx;
                length += sx;
                sx = 0;
            }
            if (sx + length > image_width)
                length = image_width - sx;
            if (length > 0) {
                const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
                while (length) {
                    int l = qMin(buffer_size, length);
                    const uint *src = op.src_fetch(src_buffer, &op, data, sy, sx, l);
                    QWS_CALLBACK_IF_NONBUFFERED(data, {
                        uint *dest = op.dest_fetch ? op.dest_fetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
                        op.func(dest, src, l, coverage);
                        if (op.dest_store)
                            op.dest_store(data->rasterBuffer, x, spans->y, dest, l);
                    },  {
                        data->rasterEngine->drawBufferSpan(src, l, x, spans->y,
                                                           l, coverage);
                    })
                    x += l;
                    sx += l;
                    length -= l;
                }
            }
        }
        ++spans;
    }
}

static void blend_untransformed_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_ARGB32_Premultiplied
        && data->texture.format != QImage::Format_RGB32) {
        blend_untransformed_generic(count, spans, userData);
        return;
    }

    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    const int image_width = data->texture.width;
    const int image_height = data->texture.height;
    int xoff = qRound(data->dx);
    int yoff = qRound(data->dy);

    while (count--) {
        int x = spans->x;
        int length = spans->len;
        int sx = xoff + x;
        int sy = yoff + spans->y;
        if (sy >= 0 && sy < image_height && sx < image_width) {
            if (sx < 0) {
                x -= sx;
                length += sx;
                sx = 0;
            }
            if (sx + length > image_width)
                length = image_width - sx;
            if (length > 0) {
                const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
                const uint *src = (uint *)data->texture.scanLine(sy) + sx;
                QWS_CALLBACK_IF_NONBUFFERED(data, {
                    uint *dest = ((uint *)data->rasterBuffer->scanLine(spans->y)) + x;
                    op.func(dest, src, length, coverage);
                }, {
                    data->rasterEngine->drawBufferSpan(src, length, x,
                                                       spans->y, length, coverage);
                })
            }
        }
        ++spans;
    }
}

#ifdef Q_WS_QWS
static void blend_untransformed_rgb16(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_RGB16 &&
        data->texture.format != QImage::Format_ARGB32_Premultiplied) {
        blend_untransformed_generic(count, spans, userData);
        return;
    }

    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    const int image_width = data->texture.width;
    const int image_height = data->texture.height;
    int xoff = qRound(data->dx);
    int yoff = qRound(data->dy);

    if (data->texture.format == QImage::Format_ARGB32_Premultiplied) {
        while (count--) {
            int coverage = (data->texture.const_alpha * spans->coverage) >> 8;
            int x = spans->x;
            int length = spans->len;
            int sx = xoff + x;
            int sy = yoff + spans->y;
            if (sy >= 0 && sy < image_height && sx < image_width) {
                if (sx < 0) {
                    x -= sx;
                    length += sx;
                    sx = 0;
                }
                if (sx + length > image_width)
                    length = image_width - sx;
                if (length > 0) {
                    ushort *dest = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + x;
                    const uint *src = (uint *)data->texture.scanLine(sy) + sx;
                    if (coverage == 255) {
                        for (int i = 0; i < length; ++i) {
                            uint s = src[i];
                            int alpha = qAlpha(s);
                            s = qConvertRgb32To16(s);
                            if (alpha != 255)
                                s += BYTE_MUL_RGB16(dest[i], 255 - alpha);
                            dest[i] = s;
                        }
                    } else {
                        for (int i = 0; i < length; ++i) {
                            uint s = src[i];
                            MASK(s, coverage);
                            int alpha = qAlpha(s);
                            s = qConvertRgb32To16(s);
                            Q_ASSERT(alpha < 255);
                            s += BYTE_MUL_RGB16(dest[i], 255 - alpha);
                            dest[i] = s;
                        }
                    }
                }
            }
            ++spans;
        }
        return;
    }

    // texture is RGB16
    while (count--) {
        int coverage = (data->texture.const_alpha * spans->coverage) >> 8;
        int x = spans->x;
        int length = spans->len;
        int sx = xoff + x;
        int sy = yoff + spans->y;
        if (sy >= 0 && sy < image_height && sx < image_width) {
            if (sx < 0) {
                x -= sx;
                length += sx;
                sx = 0;
            }
            if (sx + length > image_width)
                length = image_width - sx;
            if (length > 0) {
                ushort *dest = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + x;
                const ushort *src = (ushort *)data->texture.scanLine(sy) + sx;
                if (coverage == 255) {
                    memcpy(dest, src, length*sizeof(quint16));
                } else {
                    int ialpha = 255 - coverage;
                    for (int i = 0; i < length; ++i)
                        dest[i] = qConvertRgb32To16(INTERPOLATE_PIXEL_255(qConvertRgb16To32(src[i]), coverage,
                                                                         qConvertRgb16To32(dest[i]), ialpha));
                }
            }
        }
        ++spans;
    }
}
#endif


static void blend_tiled_generic(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    uint buffer[buffer_size];
    uint src_buffer[buffer_size];
    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    const int image_width = data->texture.width;
    const int image_height = data->texture.height;
    int xoff = qRound(data->dx) % image_width;
    int yoff = qRound(data->dy) % image_height;

    if (xoff < 0)
        xoff += image_width;
    if (yoff < 0)
        yoff += image_height;

    while (count--) {
        int x = spans->x;
        int length = spans->len;
        int sx = (xoff + spans->x) % image_width;
        int sy = (spans->y + yoff) % image_height;
        if (sx < 0)
            sx += image_width;
        if (sy < 0)
            sy += image_height;

        const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
        while (length) {
            int l = qMin(image_width - sx, length);
            if (buffer_size < l)
                l = buffer_size;
            const uint *src = op.src_fetch(src_buffer, &op, data, sy, sx, l);
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                uint *dest = op.dest_fetch ? op.dest_fetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
                op.func(dest, src, l, coverage);
                if (op.dest_store)
                    op.dest_store(data->rasterBuffer, x, spans->y, dest, l);
            }, {
                data->rasterEngine->drawBufferSpan(src, l, x, spans->y, l,
                                                   coverage);
            })
            x += l;
            sx += l;
            length -= l;
            if (sx >= image_width)
                sx = 0;
        }
        ++spans;
    }
}

static void blend_tiled_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_ARGB32_Premultiplied
        && data->texture.format != QImage::Format_RGB32) {
        blend_tiled_generic(count, spans, userData);
        return;
    }

    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    int image_width = data->texture.width;
    int image_height = data->texture.height;
    int xoff = qRound(data->dx) % image_width;
    int yoff = qRound(data->dy) % image_height;

    if (xoff < 0)
        xoff += image_width;
    if (yoff < 0)
        yoff += image_height;

    while (count--) {
        int x = spans->x;
        int length = spans->len;
        int sx = (xoff + spans->x) % image_width;
        int sy = (spans->y + yoff) % image_height;
        if (sx < 0)
            sx += image_width;
        if (sy < 0)
            sy += image_height;

        const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
        while (length) {
            int l = qMin(image_width - sx, length);
            if (buffer_size < l)
                l = buffer_size;
            const uint *src = (uint *)data->texture.scanLine(sy) + sx;
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                uint *dest = ((uint *)data->rasterBuffer->scanLine(spans->y)) + x;
                op.func(dest, src, l, coverage);
            }, {
                data->rasterEngine->drawBufferSpan(src, buffer_size,
                                                   x, spans->y, l, coverage);
            })

            x += l;
            length -= l;
            sx = 0;
        }
        ++spans;
    }
}

#ifdef Q_WS_QWS
static void blend_tiled_rgb16(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_RGB16
        && data->texture.format != QImage::Format_ARGB32_Premultiplied) {
        blend_tiled_generic(count, spans, userData);
        return;
    }

    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    int image_width = data->texture.width;
    int image_height = data->texture.height;
    int xoff = qRound(data->dx) % image_width;
    int yoff = qRound(data->dy) % image_height;

    if (xoff < 0)
        xoff += image_width;
    if (yoff < 0)
        yoff += image_height;

    if (data->texture.format == QImage::Format_ARGB32_Premultiplied) {
        while (count--) {
            int x = spans->x;
            int length = spans->len;
            int sx = (xoff + spans->x) % image_width;
            int sy = (spans->y + yoff) % image_height;
            if (sx < 0)
                sx += image_width;
            if (sy < 0)
                sy += image_height;

            if (spans->coverage == 255) {
                while (length) {
                    int l = qMin(image_width - sx, length);
                    if (buffer_size < l)
                        l = buffer_size;
                    ushort *dest = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + x;
                    const uint *src = (uint *)data->texture.scanLine(sy) + sx;
                    for (int i = 0; i < l; ++i) {
                        uint s = src[i];
                        int alpha = qAlpha(s);
                        s = qConvertRgb32To16(s);
                        if (alpha != 255)
                            s += BYTE_MUL_RGB16(dest[i], 255 - alpha);
                        dest[i] = s;
                    }
                    x += l;
                    length -= l;
                    sx = 0;
                }
            } else {
                while (length) {
                    int l = qMin(image_width - sx, length);
                    if (buffer_size < l)
                        l = buffer_size;
                    ushort *dest = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + x;
                    const uint *src = (uint *)data->texture.scanLine(sy) + sx;
                    for (int i = 0; i < l; ++i) {
                        uint s = src[i];
                        MASK(s, spans->coverage);
                        int alpha = qAlpha(s);
                        s = qConvertRgb32To16(s);
                        if (alpha != 255)
                            s += BYTE_MUL_RGB16(dest[i], 255 - alpha);
                        dest[i] = s;
                    }
                    x += l;
                    length -= l;
                    sx = 0;
                }
            }
            ++spans;
        }
        return;
    }

    // texture is RGB16
    while (count--) {
        int x = spans->x;
        int length = spans->len;
        int sx = (xoff + spans->x) % image_width;
        int sy = (spans->y + yoff) % image_height;
        if (sx < 0)
            sx += image_width;
        if (sy < 0)
            sy += image_height;

        int coverage = (data->texture.const_alpha * spans->coverage) >> 8;
        if (coverage == 255) {
            while (length) {
                int l = qMin(image_width - sx, length);
                if (buffer_size < l)
                    l = buffer_size;
                ushort *dest = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + x;
                const ushort *src = (ushort *)data->texture.scanLine(sy) + sx;
                memcpy(dest, src, l*sizeof(quint16));
                x += l;
                length -= l;
                sx = 0;
            }
        } else {
            int ialpha = 255 - coverage;
            while (length) {
                int l = qMin(image_width - sx, length);
                if (buffer_size < l)
                    l = buffer_size;
                ushort *dest = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + x;
                const ushort *src = (ushort *)data->texture.scanLine(sy) + sx;
                for (int i = 0; i < l; ++i)
                    dest[i] = qConvertRgb32To16(INTERPOLATE_PIXEL_255(qConvertRgb16To32(src[i]), coverage,
                                                                     qConvertRgb16To32(dest[i]), ialpha));
                x += l;
                length -= l;
                sx = 0;
            }
        }
        ++spans;
    }
}
#endif


static void blend_texture_generic(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);

    uint buffer[buffer_size];
    uint src_buffer[buffer_size];
    Operator op = getOperator(data, spans, count);
    if (!op.func)
        return;

    while (count--) {
        int x = spans->x;
        int length = spans->len;
        while (length) {
            int l = qMin(buffer_size, length);
            const uint *src = op.src_fetch(src_buffer, &op, data, spans->y, x, l);
            const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                uint *dest = op.dest_fetch ? op.dest_fetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
                if (op.dest_store)
                    op.dest_store(data->rasterBuffer, x, spans->y, dest, l);

                op.func(dest, src, l, coverage);
            }, {
                data->rasterEngine->drawBufferSpan(src, buffer_size,
                                                   x, spans->y, l, coverage);
            })


            x += l;
            length -= l;
        }
        ++spans;
    }
}


static void blend_transformed_bilinear_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_ARGB32_Premultiplied
        && data->texture.format != QImage::Format_RGB32) {
        blend_src_generic(count, spans, userData);
        return;
    }

    CompositionFunction func = functionForMode[data->rasterBuffer->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    while (count--) {
        void *t = data->rasterBuffer->scanLine(spans->y);

        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)data->texture.imageData;
        int x = int((data->m21 * (spans->y + 0.5)
                     + data->m11 * (spans->x + 0.5) + data->dx) * fixed_scale) - half_point;
        int y = int((data->m22 * (spans->y + 0.5)
                     + data->m12 * (spans->x + 0.5) + data->dy) * fixed_scale) - half_point;

        int length = spans->len;
        const int coverage = (data->texture.const_alpha * spans->coverage) >> 8;
        while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            while (b < end) {
                int x1 = (x >> 16);
                int x2 = x1 + 1;
                int y1 = (y >> 16);
                int y2 = y1 + 1;

                int distx = ((x - (x1 << 16)) >> 8);
                int disty = ((y - (y1 << 16)) >> 8);
                int idistx = 256 - distx;
                int idisty = 256 - disty;

                bool x1_out = ((x1 < 0) || (x1 >= image_width));
                bool x2_out = ((x2 < 0) || (x2 >= image_width));
                bool y1_out = ((y1 < 0) || (y1 >= image_height));
                bool y2_out = ((y2 < 0) || (y2 >= image_height));

                int y1_offset = y1 * image_width;
                int y2_offset = y1_offset + image_width;

#if defined(Q_IRIX_GCC3_3_WORKAROUND)
                uint tl = gccBug((x1_out || y1_out) ? uint(0) : image_bits[y1_offset + x1]);
                uint tr = gccBug((x2_out || y1_out) ? uint(0) : image_bits[y1_offset + x2]);
                uint bl = gccBug((x1_out || y2_out) ? uint(0) : image_bits[y2_offset + x1]);
                uint br = gccBug((x2_out || y2_out) ? uint(0) : image_bits[y2_offset + x2]);
#else
                uint tl = (x1_out || y1_out) ? uint(0) : image_bits[y1_offset + x1];
                uint tr = (x2_out || y1_out) ? uint(0) : image_bits[y1_offset + x2];
                uint bl = (x1_out || y2_out) ? uint(0) : image_bits[y2_offset + x1];
                uint br = (x2_out || y2_out) ? uint(0) : image_bits[y2_offset + x2];
#endif

                uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
                uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
                *b = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);
                ++b;

                x += fdx;
                y += fdy;
            }
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                func(target, buffer, l, coverage);
            }, {
                data->rasterEngine->drawBufferSpan(buffer, buffer_size,
                                                   spans->x + spans->len - length,
                                                   spans->y, l, coverage);
            })
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_transformed_bilinear_tiled_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_ARGB32_Premultiplied
        && data->texture.format != QImage::Format_RGB32) {
        blend_src_generic(count, spans, userData);
        return;
    }

    CompositionFunction func = functionForMode[data->rasterBuffer->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    while (count--) {
        void *t = data->rasterBuffer->scanLine(spans->y);

        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)data->texture.imageData;
        int x = int((data->m21 * (spans->y + 0.5)
                     + data->m11 * (spans->x + 0.5) + data->dx) * fixed_scale) - half_point;
        int y = int((data->m22 * (spans->y + 0.5)
                     + data->m12 * (spans->x + 0.5) + data->dy) * fixed_scale) - half_point;

        int length = spans->len;
        const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
        while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            while (b < end) {
                int x1 = (x >> 16);
                int x2 = (x1 + 1);
                int y1 = (y >> 16);
                int y2 = (y1 + 1);

                int distx = ((x - (x1 << 16)) >> 8);
                int disty = ((y - (y1 << 16)) >> 8);
                int idistx = 256 - distx;
                int idisty = 256 - disty;

                x1 %= image_width;
                x2 %= image_width;
                y1 %= image_height;
                y2 %= image_height;

                if (x1 < 0) x1 += image_width;
                if (x2 < 0) x2 += image_width;
                if (y1 < 0) y1 += image_height;
                if (y2 < 0) y2 += image_height;

                Q_ASSERT(x1 >= 0 && x1 < image_width);
                Q_ASSERT(x2 >= 0 && x2 < image_width);
                Q_ASSERT(y1 >= 0 && y1 < image_height);
                Q_ASSERT(y2 >= 0 && y2 < image_height);

                int y1_offset = y1 * image_width;
                int y2_offset = y2 * image_width;

#if defined(Q_IRIX_GCC3_3_WORKAROUND)
                uint tl = gccBug(image_bits[y1_offset + x1]);
                uint tr = gccBug(image_bits[y1_offset + x2]);
                uint bl = gccBug(image_bits[y2_offset + x1]);
                uint br = gccBug(image_bits[y2_offset + x2]);
#else
                uint tl = image_bits[y1_offset + x1];
                uint tr = image_bits[y1_offset + x2];
                uint bl = image_bits[y2_offset + x1];
                uint br = image_bits[y2_offset + x2];
#endif

                uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
                uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
                *b = INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);
                ++b;
                x += fdx;
                y += fdy;
            }
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                func(target, buffer, l, coverage);
            }, {
                data->rasterEngine->drawBufferSpan(buffer, buffer_size,
                                                   spans->x + spans->len - length,
                                                   spans->y, l, coverage);
            })
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_transformed_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_ARGB32_Premultiplied
        && data->texture.format != QImage::Format_RGB32) {
        blend_src_generic(count, spans, userData);
        return;
    }

    CompositionFunction func = functionForMode[data->rasterBuffer->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    while (count--) {
        void *t = data->rasterBuffer->scanLine(spans->y);

        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)data->texture.imageData;

        int x = int((data->m21 * (spans->y + 0.5)
                     + data->m11 * (spans->x + 0.5) + data->dx) * fixed_scale);
        int y = int((data->m22 * (spans->y + 0.5)
                     + data->m12 * (spans->x + 0.5) + data->dy) * fixed_scale);

        int length = spans->len;
        const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
        while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            while (b < end) {
                int px = x >> 16;
                int py = y >> 16;

                bool out = (px < 0) || (px >= image_width)
                           || (py < 0) || (py >= image_height);

                int y_offset = py * image_width;
                *b = out ? uint(0) : image_bits[y_offset + px];
                x += fdx;
                y += fdy;
                ++b;
            }
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                func(target, buffer, l, coverage);
            }, {
                data->rasterEngine->drawBufferSpan(buffer, buffer_size,
                                                   spans->x + spans->len - length,
                                                   spans->y, l, coverage);
            })
            target += l;
            length -= l;
        }
        ++spans;
    }
}

static void blend_transformed_tiled_argb(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->texture.format != QImage::Format_ARGB32_Premultiplied
        && data->texture.format != QImage::Format_RGB32) {
        blend_src_generic(count, spans, userData);
        return;
    }

    CompositionFunction func = functionForMode[data->rasterBuffer->compositionMode];
    if (!func)
        return;
    uint buffer[buffer_size];

    int image_width = data->texture.width;
    int image_height = data->texture.height;

    // The increment pr x in the scanline
    int fdx = (int)(data->m11 * fixed_scale);
    int fdy = (int)(data->m12 * fixed_scale);

    while (count--) {
        void *t = data->rasterBuffer->scanLine(spans->y);

        uint *target = ((uint *)t) + spans->x;
        uint *image_bits = (uint *)data->texture.imageData;
        int x = int((data->m21 * (spans->y + 0.5)
                     + data->m11 * (spans->x + 0.5) + data->dx) * fixed_scale);
        int y = int((data->m22 * (spans->y + 0.5)
                     + data->m12 * (spans->x + 0.5) + data->dy) * fixed_scale);

        const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
        int length = spans->len;
        while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            while (b < end) {
                int px = x >> 16;
                int py = y >> 16;
                px %= image_width;
                py %= image_height;
                if (px < 0) px += image_width;
                if (py < 0) py += image_height;
                int y_offset = py * image_width;

                Q_ASSERT(px >= 0 && px < image_width);
                Q_ASSERT(py >= 0 && py < image_height);

                *b = image_bits[y_offset + px];
                x += fdx;
                y += fdy;
                ++b;
            }
            QWS_CALLBACK_IF_NONBUFFERED(data, {
                func(target, buffer, l, coverage);
            }, {
                data->rasterEngine->drawBufferSpan(buffer, buffer_size,
                                                   spans->x + spans->len - length,
                                                   spans->y, l, coverage);
            })
            target += l;
            length -= l;
        }
        ++spans;
    }
}

/* Image formats here are target formats */
static const ProcessSpans processTextureSpans[NBlendTypes][QImage::NImageFormats] = {
    // Untransformed
    {
        0, // Invalid
      blend_untransformed_generic,   // Mono
      blend_untransformed_generic,   // MonoLsb
      blend_untransformed_generic,   // Indexed8
      blend_untransformed_generic,   // RGB32
      blend_untransformed_generic,   // ARGB32
      blend_untransformed_argb // ARGB32_Premultiplied
#ifdef Q_WS_QWS
        ,  blend_untransformed_rgb16 // RGB16
#endif
    },
    // Tiled
    {
        0, // Invalid
      blend_tiled_generic,   // Mono
      blend_tiled_generic,   // MonoLsb
      blend_tiled_generic,   // Indexed8
      blend_tiled_generic,   // RGB32
      blend_tiled_generic,   // ARGB32
        blend_tiled_argb // ARGB32_Premultiplied
#ifdef Q_WS_QWS
        ,  blend_tiled_rgb16 // RGB16
#endif
    },
    // Transformed
    {
        0, // Invalid
      blend_texture_generic,   // Mono
      blend_texture_generic,   // MonoLsb
      blend_texture_generic,   // Indexed8
      blend_texture_generic,   // RGB32
      blend_texture_generic,   // ARGB32
      blend_transformed_argb // ARGB32_Premultiplied
#ifdef Q_WS_QWS
        ,  blend_src_generic // RGB16
#endif
    },
     // TransformedTiled
    {
        0,
      blend_texture_generic,   // Mono
      blend_texture_generic,   // MonoLsb
      blend_texture_generic,   // Indexed8
      blend_texture_generic,   // RGB32
      blend_texture_generic,   // ARGB32
      blend_transformed_tiled_argb // ARGB32_Premultiplied
#ifdef Q_WS_QWS
        ,  blend_src_generic // RGB16
#endif
    },
    // Bilinear
    {
        0,
      blend_texture_generic,   // Mono
      blend_texture_generic,   // MonoLsb
      blend_texture_generic,   // Indexed8
      blend_texture_generic,   // RGB32
      blend_texture_generic,   // ARGB32
      blend_transformed_bilinear_argb // ARGB32_Premultiplied
#ifdef Q_WS_QWS
        ,  blend_src_generic // RGB16
#endif
    },
    // BilinearTiled
    {
        0,
      blend_texture_generic,   // Mono
      blend_texture_generic,   // MonoLsb
      blend_texture_generic,   // Indexed8
      blend_texture_generic,   // RGB32
      blend_texture_generic,   // ARGB32
      blend_transformed_bilinear_tiled_argb // ARGB32_Premultiplied
#ifdef Q_WS_QWS
        ,  blend_src_generic // RGB16
#endif
    }
};

void qBlendTexture(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    ProcessSpans proc = processTextureSpans[getBlendType(data)][data->rasterBuffer->format];
    proc(count, spans, userData);
}


DrawHelper qDrawHelper[QImage::NImageFormats] =
{
    // Format_Invalid,
    { 0, 0 },
    // Format_Mono,
    {
        blend_color_generic,
        blend_src_generic,
    },
    // Format_MonoLSB,
    {
        blend_color_generic,
        blend_src_generic,
    },
    // Format_Indexed8,
    {
        blend_color_generic,
        blend_src_generic,
    },
    // Format_RGB32,
    {
        blend_color_generic,
        blend_src_generic,
    },
    // Format_ARGB32,
    {
        blend_color_generic,
        blend_src_generic,
    },
    // Format_ARGB32_Premultiplied
    {
        blend_color_argb,
        blend_src_argb,
    }
#ifdef Q_WS_QWS
    ,
    { // Format_RGB16
        blend_color_rgb16,
        blend_src_generic
    }
#endif
};



#if (defined(QT_HAVE_SSE) && (!defined(__APPLE__) || defined(__i386__))) || defined(QT_HAVE_IWMMXT)

enum CPUFeatures {
    None = 0,
    MMX = 0x1,
    SSE = 0x2,
    SSE2 = 0x4,
    CMOV = 0x8
};

static uint detectCPUFeatures() {
#if defined(__x86_64__) || defined(Q_OS_WIN64)
    return MMX|SSE|SSE2|CMOV;
#elif defined(__ia64__)
    return MMX|SSE|SSE2;
#elif defined(__IWMMXT__)
    return MMX|SSE;
#else
    uint result = 0;
    /* see p. 118 of amd64 instruction set manual Vol3 */
#if defined(Q_CC_GNU)
    asm ("push %%ebx\n"
         "pushf\n"
         "pop %%eax\n"
         "mov %%eax, %%ebx\n"
         "xor $0x00200000, %%eax\n"
         "push %%eax\n"
         "popf\n"
         "pushf\n"
         "pop %%eax\n"
         "xor %%edx, %%edx\n"
         "xor %%ebx, %%eax\n"
         "jz 1f\n"

         "mov $0x00000001, %%eax\n"
         "cpuid\n"
         "1:\n"
         "pop %%ebx\n"
         "mov %%edx, %0\n"
        : "=r" (result)
        :
        : "%eax", "%ecx", "%edx"
        );
#elif defined (Q_OS_WIN)
    _asm {
	push eax
	push ebx
	push ecx
	push edx
	pushfd
	pop eax
	mov ebx, eax
	xor eax, 00200000h
	push eax
	popfd
	pushfd
        pop eax
	mov edx, 0
	xor eax, ebx
	jz skip

	mov eax, 1
	cpuid
	mov result, edx
    skip:
        pop edx
	pop ecx
	pop ebx
	pop eax
    }
#endif
    uint features = 0;
    // result now contains the standard feature bits
    if (result & (1 << 15))
        features |= CMOV;
    if (result & (1 << 23))
        features |= MMX;
    if (result & (1 << 25))
        features |= SSE;
    if (result & (1 << 26))
        features |= SSE2;
    return features;
#endif
}

extern void qt_blend_color_argb_sse(int count, const QSpan *spans, void *userData);

#ifdef Q_WS_QWS
void qResetDrawhelper()
{
    functionForMode = functionForMode_C;
    functionForModeSolid = functionForModeSolid_C;
    qDrawHelper[QImage::Format_ARGB32_Premultiplied].blendColor = blend_color_argb;
}

void qInitDrawhelperAsm()
{
    static uint features = 0xffffffff;
    if (features == 0xffffffff)
        features = detectCPUFeatures();

#ifdef QT_NO_DEBUG
    if (features & SSE) {
        functionForMode = qt_functionForMode_SSE;
        functionForModeSolid = qt_functionForModeSolid_SSE;
        qDrawHelper[QImage::Format_ARGB32_Premultiplied].blendColor = qt_blend_color_argb_sse;
    } else {
        qResetDrawhelper();
    }
#endif
}
#else
void qInitDrawhelperAsm()
{
    static uint features = 0xffffffff;
    if (features != 0xffffffff)
        return;
    features = detectCPUFeatures();

#ifdef QT_NO_DEBUG
    if (features & SSE) {
        functionForMode = qt_functionForMode_SSE;
        functionForModeSolid = qt_functionForModeSolid_SSE;
        qDrawHelper[QImage::Format_ARGB32_Premultiplied].blendColor = qt_blend_color_argb_sse;
    }
#endif
}
#endif // Q_WS_QWS

#else

void qInitDrawhelperAsm() {}
#ifdef Q_WS_QWS
void qResetDrawhelper() {}
#endif

#endif // Q_HAVE_SSE

