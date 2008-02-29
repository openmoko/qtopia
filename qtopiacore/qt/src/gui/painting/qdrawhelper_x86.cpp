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

#if (defined(QT_HAVE_SSE) && (!defined(__APPLE__) || defined(__i386__))) || defined(QT_HAVE_IWMMXT)

#include <mmintrin.h>
#if !defined(__IWMMXT__)
#  include <xmmintrin.h>
#endif
#define C_FF const m64 mmx_0x00ff = _mm_set1_pi16(0xff)
#define C_80 const m64 mmx_0x0080 = _mm_set1_pi16(0x80)
#define C_00 const m64 mmx_0x0000 = _mm_setzero_si64()
#if defined(Q_OS_WIN)
#  pragma warning(disable: 4799) // No EMMS at end of function
#endif

typedef __m64 m64;

#ifndef _MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) \
 (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | (fp0))

#endif

static inline m64 alpha(m64 x)
{
    return _mm_shuffle_pi16 (x, _MM_SHUFFLE(3, 3, 3, 3));
}

static inline m64 _negate(const m64 &x, const m64 &mmx_0x00ff)
{
    return _mm_xor_si64(x, mmx_0x00ff);
}
#define negate(x) _negate(x, mmx_0x00ff)

static inline m64 add(const m64 &a, const m64 &b)
{
    return  _mm_adds_pu16 (a, b);
}

static inline m64 _byte_mul(const m64 &a, const m64 &b, const m64 &mmx_0x0080)
{
    m64 res = _mm_mullo_pi16(a, b);
    res = _mm_adds_pu16(res, mmx_0x0080);
    res = _mm_adds_pu16(res, _mm_srli_pi16 (res, 8));
    return _mm_srli_pi16(res, 8);
}
#define byte_mul(a, b) _byte_mul(a, b, mmx_0x0080)

static inline m64 interpolate_pixel_256(const m64 &x, const m64 &a, const m64 &y, const m64 &b) {
    m64 res = _mm_adds_pu16(_mm_mullo_pi16(x, a), _mm_mullo_pi16(y, b));
    return _mm_srli_pi16(res, 8);
}

static inline m64 _interpolate_pixel_255(const m64 &x, const m64 &a, const m64 &y, const m64 &b, const m64 &mmx_0x0080) {
    m64 res = _mm_adds_pu16(_mm_mullo_pi16(x, a), _mm_mullo_pi16(y, b));
    res = _mm_adds_pu16(res, mmx_0x0080);
    res = _mm_adds_pu16(res, _mm_srli_pi16 (res, 8));
    return _mm_srli_pi16(res, 8);
}
#define interpolate_pixel_255(x, a, y, b) _interpolate_pixel_255(x, a, y, b, mmx_0x0080)

static inline m64 _premul(m64 x, const m64 &mmx_0x0080) {
    m64 a = alpha(x);
    return _byte_mul(x, a, mmx_0x0080);
}
#define premul(x) _premul(x, mmx_0x0080)

static inline m64 _load(uint x, const m64 &mmx_0x0000)
{
    return _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), mmx_0x0000);
}
#define load(x) _load(x, mmx_0x0000)

static inline m64 _load_alpha(uint x, const m64 &mmx_0x0000)
{
    m64 t = _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), mmx_0x0000);
    return _mm_shuffle_pi16 (t, _MM_SHUFFLE(0, 0, 0, 0));
}
#define load_alpha(x) _load_alpha(x, mmx_0x0000)

static inline uint _store(const m64 &x, const m64 &mmx_0x0000)
{
    return _mm_cvtsi64_si32(_mm_packs_pu16(x, mmx_0x0000));
}
#define store(x) _store(x, mmx_0x0000)

#if defined(__IWMMXT__)
static inline void end_mmx() {}
#else
static inline void end_mmx()
{
    _mm_empty();
}
#endif

/*
  result = 0
  d = d * cia
*/
static void QT_FASTCALL comp_func_solid_Clear(uint *dest, int length, uint, uint const_alpha)
{
    if (!length)
        return;

    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, 0);
    } else {
        C_FF; C_80; C_00;
        m64 ia = negate(load_alpha(const_alpha));
        for (int i = 0; i < length; ++i) {
            dest[i] = store(byte_mul(load(dest[i]), ia));
        }
    }
    end_mmx();
}

static void QT_FASTCALL comp_func_Clear(uint *dest, const uint *, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, 0);
    } else {
        C_FF; C_80; C_00;
        m64 ia = negate(load_alpha(const_alpha));
        for (int i = 0; i < length; ++i)
            dest[i] = store(byte_mul(load(dest[i]), ia));
    }
    end_mmx();
}

/*
  result = s
  dest = s * ca + d * cia
*/
static void QT_FASTCALL comp_func_solid_Source(uint *dest, int length, uint src, uint const_alpha)
{
    if (const_alpha == 255) {
        QT_MEMFILL_UINT(dest, length, src);
    } else {
        C_FF; C_80; C_00;
        const m64 a = load_alpha(const_alpha);
        const m64 ia = negate(a);
        const m64 s = byte_mul(load(src), a);
        for (int i = 0; i < length; ++i) {
            dest[i] = store(add(s, byte_mul(load(dest[i]), ia)));
        }
        end_mmx();
    }
}

static void QT_FASTCALL comp_func_Source(uint *dest, const uint *src, int length, uint const_alpha)
{
    if (const_alpha == 255) {
        ::memcpy(dest, src, length * sizeof(uint));
    } else {
        C_FF; C_80; C_00;
        const m64 a = load_alpha(const_alpha);
        const m64 ia = negate(a);
        for (int i = 0; i < length; ++i)
            dest[i] = store(interpolate_pixel_255(load(src[i]), a, load(dest[i]), ia));
    }
    end_mmx();
}

/*
  result = s + d * sia
  dest = (s + d * sia) * ca + d * cia
       = s * ca + d * (sia * ca + cia)
       = s * ca + d * (1 - sa*ca)
*/
static void QT_FASTCALL comp_func_solid_SourceOver(uint *dest, int length, uint src, uint const_alpha)
{
    if ((const_alpha & qAlpha(src)) == 255) {
        QT_MEMFILL_UINT(dest, length, src);
    } else {
        C_FF; C_80; C_00;
        m64 s = load(src);
        if (const_alpha != 255) {
            m64 ca = load_alpha(const_alpha);
            s = byte_mul(s, ca);
        }
        m64 a = negate(alpha(s));
        for (int i = 0; i < length; ++i)
            dest[i] = store(add(s, byte_mul(load(dest[i]), a)));
        end_mmx();
    }
}

static void QT_FASTCALL comp_func_SourceOver(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF;
    C_80;
    C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = load(src[i]);
            m64 ia = negate(alpha(s));
            dest[i] = store(add(s, byte_mul(load(dest[i]), ia)));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = byte_mul(load(src[i]), ca);
            m64 ia = negate(alpha(s));
            dest[i] = store(add(s, byte_mul(load(dest[i]), ia)));
        }
    }
    end_mmx();
}

/*
  result = d + s * dia
  dest = (d + s * dia) * ca + d * cia
       = d + s * dia * ca
*/
static void QT_FASTCALL comp_func_solid_DestinationOver(uint *dest, int length, uint src, uint const_alpha)
{
    C_FF; C_80; C_00;
    m64 s = load(src);
    if (const_alpha != 255)
        s = byte_mul(s, load_alpha(const_alpha));

    for (int i = 0; i < length; ++i) {
        m64 d = load(dest[i]);
        m64 dia = negate(alpha(d));
        dest[i] = store(add(d, byte_mul(s, dia)));
    }
    end_mmx();
}

static void QT_FASTCALL comp_func_DestinationOver(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            m64 ia = negate(alpha(d));
            dest[i] = store(add(d, byte_mul(load(src[i]), ia)));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            m64 dia = negate(alpha(d));
            dia = byte_mul(dia, ca);
            dest[i] = store(add(d, byte_mul(load(src[i]), dia)));
        }
    }
    end_mmx();
}

/*
  result = s * da
  dest = s * da * ca + d * cia
*/
static void QT_FASTCALL comp_func_solid_SourceIn(uint *dest, int length, uint src, uint const_alpha)
{
    C_80; C_00;
    if (const_alpha == 255) {
        m64 s = load(src);
        for (int i = 0; i < length; ++i) {
            m64 da = alpha(load(dest[i]));
            dest[i] = store(byte_mul(s, da));
        }
    } else {
        C_FF;
        m64 s = load(src);
        m64 ca = load_alpha(const_alpha);
        s = byte_mul(s, ca);
        m64 cia = negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            dest[i] = store(interpolate_pixel_255(s, alpha(d), d, cia));
        }
    }
    end_mmx();
}

static void QT_FASTCALL comp_func_SourceIn(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 a = alpha(load(dest[i]));
            dest[i] = store(byte_mul(load(src[i]), a));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        m64 cia = negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            m64 da = byte_mul(alpha(d), ca);
            dest[i] = store(interpolate_pixel_255(load(src[i]), da, d, cia));
        }
    }
    end_mmx();
}

/*
  result = d * sa
  dest = d * sa * ca + d * cia
       = d * (sa * ca + cia)
*/
static void QT_FASTCALL comp_func_solid_DestinationIn(uint *dest, int length, uint src, uint const_alpha)
{
    C_80; C_00;
    m64 a = alpha(load(src));
    if (const_alpha != 255) {
        C_FF;
        m64 ca = load_alpha(const_alpha);
        m64 cia = negate(ca);
        a = byte_mul(a, ca);
        a = add(a, cia);
    }
    for (int i = 0; i < length; ++i)
        dest[i] = store(byte_mul(load(dest[i]), a));
    end_mmx();
}

static void QT_FASTCALL comp_func_DestinationIn(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 a = alpha(load(src[i]));
            dest[i] = store(byte_mul(load(dest[i]), a));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        m64 cia = negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            m64 a = alpha(load(src[i]));
            a = byte_mul(a, ca);
            a = add(a, cia);
            dest[i] = store(byte_mul(d, a));
        }
    }
    end_mmx();
}

/*
  result = s * dia
  dest = s * dia * ca + d * cia
*/
static void QT_FASTCALL comp_func_solid_SourceOut(uint *dest, int length, uint src, uint const_alpha)
{
    C_FF; C_80; C_00;
    m64 s = load(src);
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 dia = negate(alpha(load(dest[i])));
            dest[i] = store(byte_mul(s, dia));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        m64 cia = negate(ca);
        s = byte_mul(s, ca);
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            dest[i] = store(interpolate_pixel_255(s, negate(alpha(d)), d, cia));
        }
    }
    end_mmx();
}

static void QT_FASTCALL comp_func_SourceOut(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 ia = negate(alpha(load(dest[i])));
            dest[i] = store(byte_mul(load(src[i]), ia));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        m64 cia = negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            m64 dia = byte_mul(negate(alpha(d)), ca);
            dest[i] = store(interpolate_pixel_255(load(src[i]), dia, d, cia));
        }
    }
    end_mmx();
}

/*
  result = d * sia
  dest = d * sia * ca + d * cia
       = d * (sia * ca + cia)
*/
static void QT_FASTCALL comp_func_solid_DestinationOut(uint *dest, int length, uint src, uint const_alpha)
{
    C_FF; C_80; C_00;
    m64 a = negate(alpha(load(src)));
    if (const_alpha != 255) {
        m64 ca = load_alpha(const_alpha);
        a = byte_mul(a, ca);
        a = add(a, negate(ca));
    }
    for (int i = 0; i < length; ++i)
        dest[i] = store(byte_mul(load(dest[i]), a));
    end_mmx();
}

static void QT_FASTCALL comp_func_DestinationOut(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 a = negate(alpha(load(src[i])));
            dest[i] = store(byte_mul(load(dest[i]), a));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        m64 cia = negate(ca);
        for (int i = 0; i < length; ++i) {
            m64 d = load(dest[i]);
            m64 a = negate(alpha(load(src[i])));
            a = byte_mul(a, ca);
            a = add(a, cia);
            dest[i] = store(byte_mul(d, a));
        }
    }
    end_mmx();
}

/*
  result = s*da + d*sia
  dest = s*da*ca + d*sia*ca + d *cia
       = s*ca * da + d * (sia*ca + cia)
       = s*ca * da + d * (1 - sa*ca)
*/
static void QT_FASTCALL comp_func_solid_SourceAtop(uint *dest, int length, uint src, uint const_alpha)
{
    C_FF; C_80; C_00;
    m64 s = load(src);
    if (const_alpha != 255) {
        m64 ca = load_alpha(const_alpha);
        s = byte_mul(s, ca);
    }
    m64 a = negate(alpha(s));
    for (int i = 0; i < length; ++i) {
        m64 d = load(dest[i]);
        dest[i] = store(interpolate_pixel_255(s, alpha(d), d, a));
    }
    end_mmx();
}

static void QT_FASTCALL comp_func_SourceAtop(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = load(src[i]);
            m64 d = load(dest[i]);
            dest[i] = store(interpolate_pixel_255(s, alpha(d), d, negate(alpha(s))));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = load(src[i]);
            s = byte_mul(s, ca);
            m64 d = load(dest[i]);
            dest[i] = store(interpolate_pixel_255(s, alpha(d), d, negate(alpha(s))));
        }
    }
    end_mmx();
}

/*
  result = d*sa + s*dia
  dest = d*sa*ca + s*dia*ca + d *cia
       = s*ca * dia + d * (sa*ca + cia)
*/
static void QT_FASTCALL comp_func_solid_DestinationAtop(uint *dest, int length, uint src, uint const_alpha)
{
    C_FF; C_80; C_00;
    m64 s = load(src);
    m64 a = alpha(s);
    if (const_alpha != 255) {
        m64 ca = load_alpha(const_alpha);
        s = byte_mul(s, ca);
        a = alpha(s);
        a = add(a, negate(ca));
    }
    for (int i = 0; i < length; ++i) {
        m64 d = load(dest[i]);
        dest[i] = store(interpolate_pixel_255(s, negate(alpha(d)), d, a));
    }
    end_mmx();
}

static void QT_FASTCALL comp_func_DestinationAtop(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = load(src[i]);
            m64 d = load(dest[i]);
            dest[i] = store(interpolate_pixel_255(d, alpha(s), s, negate(alpha(d))));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = load(src[i]);
            s = byte_mul(s, ca);
            m64 d = load(dest[i]);
            m64 a = alpha(s);
            a = add(a, negate(ca));
            dest[i] = store(interpolate_pixel_255(s, negate(alpha(d)), d, a));
        }
    }
    end_mmx();
}

/*
  result = d*sia + s*dia
  dest = d*sia*ca + s*dia*ca + d *cia
       = s*ca * dia + d * (sia*ca + cia)
       = s*ca * dia + d * (1 - sa*ca)
*/
static void QT_FASTCALL comp_func_solid_XOR(uint *dest, int length, uint src, uint const_alpha)
{
    C_FF; C_80; C_00;
    m64 s = load(src);
    if (const_alpha != 255) {
        m64 ca = load_alpha(const_alpha);
        s = byte_mul(s, ca);
    }
    m64 a = negate(alpha(s));
    for (int i = 0; i < length; ++i) {
        m64 d = load(dest[i]);
        dest[i] = store(interpolate_pixel_255(s, negate(alpha(d)), d, a));
    }
    end_mmx();
}

static void QT_FASTCALL comp_func_XOR(uint *dest, const uint *src, int length, uint const_alpha)
{
    C_FF; C_80; C_00;
    if (const_alpha == 255) {
        for (int i = 0; i < length; ++i) {
            m64 s = load(src[i]);
            m64 d = load(dest[i]);
            dest[i] = store(interpolate_pixel_255(s, negate(alpha(d)), d, negate(alpha(s))));
        }
    } else {
        m64 ca = load_alpha(const_alpha);
        for (int i = 0; i < length; ++i) {
            m64 s = load(src[i]);
            s = byte_mul(s, ca);
            m64 d = load(dest[i]);
            dest[i] = store(interpolate_pixel_255(s, negate(alpha(d)), d, negate(alpha(s))));
        }
    }
    end_mmx();
}

const CompositionFunctionSolid qt_functionForModeSolid_SSE[] = {
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

const CompositionFunction qt_functionForMode_SSE[] = {
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


void qt_blend_color_argb_sse(int count, const QSpan *spans, void *userData)
{
    QSpanData *data = reinterpret_cast<QSpanData *>(userData);
    if (data->rasterBuffer->compositionMode == QPainter::CompositionMode_Source
        || (data->rasterBuffer->compositionMode == QPainter::CompositionMode_SourceOver
            && qAlpha(data->solid.color) == 255)) {
        // inline for performance
        C_FF;
        C_80;
        C_00;
        while (count--) {
            uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
            if (spans->coverage == 255) {
                QT_MEMFILL_UINT(target, spans->len, data->solid.color);
            } else {
                // dest = s * ca + d * (1 - sa*ca) --> dest = s * ca + d * (1-ca)
                m64 ca = load_alpha(spans->coverage);
                m64 s = byte_mul(load(data->solid.color), ca);
                m64 ica = negate(ca);
                for (int i = 0; i < spans->len; ++i)
                    target[i] = store(add(s, byte_mul(load(target[i]), ica)));
            }
            ++spans;
        }
        end_mmx();
        return;
    }
    CompositionFunctionSolid func = qt_functionForModeSolid_SSE[data->rasterBuffer->compositionMode];
    if (!func)
        return;

    while (count--) {
        uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
        func(target, spans->len, data->solid.color, spans->coverage);
        ++spans;
    }
}

#endif //QT_HAVE_SSE
