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


#ifndef QT_NO_QWS_TRANSFORMED

#include "qscreentransformed_qws.h"
#include <qscreendriverfactory_qws.h>
#include <qvector.h>
#include <private/qpainter_p.h>
#include <qmatrix.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <qwindowsystem_qws.h>
#include <qwsdisplay_qws.h>

#define QT_ROTATION_CACHEDREAD 1
#define QT_ROTATION_CACHEDWRITE 2
#define QT_ROTATION_PACKING 3
#define QT_ROTATION_TILED 4

#ifndef QT_ROTATION_ALGORITHM
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#define QT_ROTATION_ALGORITHM QT_ROTATION_TILED
#else
#define QT_ROTATION_ALGORITHM QT_ROTATION_CACHEDREAD
#endif
#endif

#if QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
static const int tileSize = 32;
#endif

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#if QT_ROTATION_ALGORITHM == QT_ROTATION_PACKED || QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
#error Big endian version not implemented for the transformed driver!
#endif
#endif

//#define QT_REGION_DEBUG

class QTransformedScreenPrivate
{
public:
    QTransformedScreenPrivate(QTransformedScreen *parent)
        : transformation(QTransformedScreen::None), subscreen(0), q(parent) {}

    void configure();

    QTransformedScreen::Transformation transformation;
    QScreen *subscreen;
    QTransformedScreen *q;
};


// Global function -----------------------------------------------------------
static QTransformedScreen *qt_trans_screen = 0;

void qws_setScreenTransformation(int t)
{
    if (qt_trans_screen) {
        qt_trans_screen->setTransformation((QTransformedScreen::Transformation)t);
        qwsServer->refresh();
    }
}

// ---------------------------------------------------------------------------
// Transformed Screen
// ---------------------------------------------------------------------------

/*!
    \internal

    \class QTransformedScreen
    \ingroup qws

    \brief The QTransformedScreen class implements a screen driver for
    a transformed screen.

    Note that this class is only available in \l {Qtopia Core}.
    Custom screen drivers can be added by subclassing the
    QScreenDriverPlugin class, using the QScreenDriverFactory class to
    dynamically load the driver into the application, but there should
    only be one screen object per application.

    Use the QScreen::isTransformed() function to determine if a screen
    is transformed. The QTransformedScreen class itself provides means
    of rotating the screen with its setTransformation() function; the
    transformation() function returns the currently set rotation in
    terms of the \l Transformation enum (which describes the various
    available rotation settings). Alternatively, QTransformedScreen
    provides an implementation of the QScreen::transformOrientation()
    function, returning the current rotation as an integer value.

    \sa QScreen, QScreenDriverPlugin, {Running Applications}
*/

/*!
    \enum QTransformedScreen::Transformation

    This enum describes the various rotations a transformed screen can
    have.

    \value None No rotation
    \value Rot90 90 degrees rotation
    \value Rot180 180 degrees rotation
    \value Rot270 270 degrees rotation
*/

/*!
    \fn bool QTransformedScreen::isTransformed() const
    \reimp
*/

/*!
    Constructs a QTransformedScreen object. The \a displayId argument
    identifies the Qtopia Core server to connect to.
*/
QTransformedScreen::QTransformedScreen(int displayId)
    : QScreen(displayId)
{
    d_ptr = new QTransformedScreenPrivate(this);
    d_ptr->transformation = None;
    qt_trans_screen = this;

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::QTransformedScreen";
#endif
}

void QTransformedScreenPrivate::configure()
{
    q->d = subscreen->depth();
    q->w = subscreen->width();
    q->h = subscreen->height();
    q->dw = subscreen->deviceWidth();
    q->dh = subscreen->deviceHeight();
    q->lstep = subscreen->linestep();
    q->data = subscreen->base();
    q->lstep = subscreen->linestep();
    q->size = subscreen->screenSize();

    q->setOffset(subscreen->offset());
    // ###: works because setTransformation recalculates unconditionally
    q->setTransformation(transformation);
}

/*!
    Destroys the QTransformedScreen object.
*/
QTransformedScreen::~QTransformedScreen()
{
    delete d_ptr;
}

static int getDisplayId(const QString &spec)
{
    QRegExp regexp(":(\\d+)\\b");
    if (regexp.lastIndexIn(spec) != -1) {
        const QString capture = regexp.cap(1);
        return capture.toInt();
    }
    return 0;
}

static QTransformedScreen::Transformation filterTransformation(QString &spec)
{
    QRegExp regexp("\\bRot(\\d+):?\\b", Qt::CaseInsensitive);
    if (regexp.indexIn(spec) == -1)
        return QTransformedScreen::None;

    const int degrees = regexp.cap(1).toInt();
    spec.remove(regexp.pos(0), regexp.matchedLength());

    return static_cast<QTransformedScreen::Transformation>(degrees / 90);
}

/*!
    \reimp
*/
bool QTransformedScreen::connect(const QString &displaySpec)
{
    QString dspec = displaySpec.trimmed();
    if (dspec.startsWith("Transformed:", Qt::CaseInsensitive))
        dspec = dspec.mid(QString("Transformed:").size());
    else if (!dspec.compare(QLatin1String("Transformed"), Qt::CaseInsensitive))
        dspec = QString();

    const QString displayIdSpec = QString(" :%1").arg(displayId);
    if (dspec.endsWith(displayIdSpec))
        dspec = dspec.left(dspec.size() - displayIdSpec.size());

    d_ptr->transformation = filterTransformation(dspec);

    QString driver = dspec;
    int colon = driver.indexOf(':');
    if (colon >= 0)
        driver.truncate(colon);

    if (!QScreenDriverFactory::keys().contains(driver, Qt::CaseInsensitive))
        if (!dspec.isEmpty())
            dspec.prepend(":");

    const int id = getDisplayId(dspec);
    d_ptr->subscreen = qt_get_screen(id, dspec.toLatin1().constData());
    d_ptr->configure();

    // XXX
    qt_screen = this;

    return true;
}

/*!
    Returns the currently set rotation.

    \sa setTransformation(), QScreen::transformOrientation()
*/
QTransformedScreen::Transformation QTransformedScreen::transformation() const
{
    return d_ptr->transformation;
}

/*!
    \reimp
*/
int QTransformedScreen::transformOrientation() const
{
    return (int)d_ptr->transformation;
}

/*!
    Rotates this screen object according to the specified \a transformation.

    \sa transformation()
*/
void QTransformedScreen::setTransformation(Transformation transformation)
{
    d_ptr->transformation = transformation;
    QSize s = mapFromDevice(QSize(dw, dh));
    w = s.width();
    h = s.height();

    const QScreen *screen = d_ptr->subscreen;
    s = mapFromDevice(QSize(screen->physicalWidth(), screen->physicalHeight()));
    physWidth = s.width();
    physHeight = s.height();

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::setTransformation" << transformation
             << "size" << w << h << "dev size" << dw << dh;
#endif

}

static inline QRect correctNormalized(const QRect &r) {
    const int x1 = qMin(r.left(), r.right());
    const int x2 = qMax(r.left(), r.right());
    const int y1 = qMin(r.top(), r.bottom());
    const int y2 = qMax(r.top(), r.bottom());

    return QRect( QPoint(x1,y1), QPoint(x2,y2) );
}

template <class SRC, class DST>
static inline DST colorConvert(SRC color)
{
    return color;
}

#if defined(QT_QWS_DEPTH_16) && defined(QT_QWS_DEPTH_32)
template <>
static inline quint32 colorConvert(quint16 color)
{
    return qt_conv16ToRgb(color);
}
#endif

#ifdef QT_QWS_DEPTH_16
template <>
static inline quint16 colorConvert(quint32 color)
{
    return qt_convRgbTo16(color);
}
#endif

#ifdef QT_QWS_DEPTH_8
template <>
static inline quint8 colorConvert(quint32 color)
{
    uchar r = (qRed(color) + 0x19) / 0x33;
    uchar g = (qGreen(color) + 0x19) / 0x33;
    uchar b = (qBlue(color) + 0x19) / 0x33;

    return r*6*6 + g*6 + b;
}

template <>
static inline quint8 colorConvert(quint16 color)
{
    return colorConvert<quint32, quint8>(qt_conv16ToRgb(color));
}
#endif // QT_QWS_DEPTH_8

#ifdef QT_QWS_DEPTH_24

// XXX: endianess??
class quint24
{
public:
    quint24(quint32 v)
    {
        data[0] = v & 0xff;
        data[1] = (v >> 8) & 0xff;
        data[2] = (v >> 16) & 0xff;
    }

private:
    uchar data[3];
} Q_PACKED;

template<>
static inline quint24 colorConvert(quint32 color)
{
    return quint24(color);
}

#endif // QT_QWS_DEPTH_24

#ifdef QT_QWS_DEPTH_18

// XXX: endianess??
class quint18
{
public:
    quint18(quint32 v)
    {
        uchar b = v & 0xff;
        uchar g = (v >> 8) & 0xff;
        uchar r = (v >> 16) & 0xff;
        uint p = (b>>2) | ((g>>2) << 6) | ((r>>2) << 12);
        data[0] = p & 0xff;
        data[1] = (p >> 8) & 0xff;
        data[2] = (p >> 16) & 0xff;
    }

private:
    uchar data[3];
} Q_PACKED;

template<>
static inline quint18 colorConvert(quint32 color)
{
    return quint18(color);
}

#endif // QT_QWS_DEPTH_18

typedef void (*BlitFunc)(QScreen *, const QImage &, const QRect &, const QPoint &);

#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD || defined(QT_QWS_DEPTH_18) || defined(QT_QWS_DEPTH_24)

template <class SRC, class DST>
static inline void blit90_cachedRead(QScreen *screen, const QImage &image,
                                     const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    for (int y = 0; y < h; ++y) {
        for (int x = w - 1; x >= 0; --x) {
            dest[(w - x - 1) * dstride + y] = colorConvert<SRC,DST>(src[x]);
        }
        src += sstride;
    }
}

template <class SRC, class DST>
static void blit270_cachedRead(QScreen *screen, const QImage &image,
                               const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits()) +
                      rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    src += (h - 1) * sstride;
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            dest[x * dstride + h - y - 1] = colorConvert<SRC,DST>(src[x]);
        }
        src -= sstride;
    }
}

#endif // QT_ROTATION_CACHEDREAD

#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE

template <class SRC, class DST>
static inline void blit90_cachedWrite(QScreen *screen, const QImage &image,
                                      const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    for (int x = w - 1; x >= 0; --x) {
        DST *d = dest + (w - x - 1) * dstride;
        for (int y = 0; y < h; ++y) {
            *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
        }
    }

}

template <class SRC, class DST>
static void blit270_cachedWrite(QScreen *screen, const QImage &image,
                                const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits()) +
                      rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    for (int x = 0; x < w; ++x) {
        DST *d = dest + x * dstride;
        for (int y = h - 1; y >= 0; --y) {
            *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
        }
    }
}

#endif // QT_ROTATION_CACHEDWRITE

#if QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING

template <class SRC, class DST>
static inline void blit90_packing(QScreen *screen, const QImage &image,
                                  const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = (quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST);

    for (int x = w - 1; x >= 0; --x) {
        int y = 0;

        for (int i = 0; i < unaligned; ++i) {
            dest[(w - x - 1) * dstride + y]
                = colorConvert<SRC,DST>(src[y * sstride + x]);
            ++y;
        }

        quint32 *d = reinterpret_cast<quint32*>(dest + (w - x - 1) * dstride
                                                + unaligned);
        const int rest = (h - unaligned) % pack;
        while (y < h - rest) {
            quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
            for (int i = 1; i < pack; ++i) {
                c |= colorConvert<SRC,DST>(src[(y + i) * sstride + x])
                     << (sizeof(int) * 8 / pack * i);
            }
            *d++ = c;
            y += pack;
        }

        while (y < h) {
            dest[(w - x - 1) * dstride + y]
                 = colorConvert<SRC,DST>(src[y * sstride + x]);
            ++y;
        }
    }
}

template <class SRC, class DST>
static inline void blit270_packing(QScreen *screen, const QImage &image,
                                   const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits()) +
                      rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = (quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST);

    for (int x = 0; x < w; ++x) {
        int y = h - 1;

        for (int i = 0; i < unaligned; ++i) {
            dest[x * dstride + h - y - 1]
                = colorConvert<SRC,DST>(src[y * sstride + x]);
            --y;
        }

        quint32 *d = reinterpret_cast<quint32*>(dest + x * dstride
                                                + unaligned);
        const int rest = (h - unaligned) % pack;
        while (y > rest) {
            quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
            for (int i = 1; i < pack; ++i) {
                c |= colorConvert<SRC,DST>(src[(y - i) * sstride + x])
                     << (sizeof(int) * 8 / pack * i);
            }
            *d++ = c;
            y -= pack;
        }
        while (y >= 0) {
            dest[x * dstride + h - y - 1]
                = colorConvert<SRC,DST>(src[y * sstride + x]);
            --y;
        }
    }
}

#endif // QT_ROTATION_PACKING

#if QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
template <class SRC, class DST>
static inline void blit90_tiled(QScreen *screen, const QImage &image,
                                const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned =
        qMin((quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST), uint(h));
    const int restX = w % tileSize;
    const int restY = (h - unaligned) % tileSize;
    const int unoptimizedY = restY % pack;
    const int numTilesX = w / tileSize + (restX > 0);
    const int numTilesY = (h - unaligned) / tileSize + (restY >= pack);

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = w - tx * tileSize - 1;
        const int stopx = qMax(startx - tileSize, 0);

        if (unaligned) {
            for (int x = startx; x >= stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride;
                for (int y = 0; y < unaligned; ++y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = ty * tileSize + unaligned;
            const int stopy = qMin(starty + tileSize, h - unoptimizedY);

            for (int x = startx; x >= stopx; --x) {
                quint32 *d = reinterpret_cast<quint32*>(dest + (w - x - 1) * dstride + starty);
                for (int y = starty; y < stopy; y += pack) {
                    quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
                    for (int i = 1; i < pack; ++i) {
                        const int shift = (sizeof(int) * 8 / pack * i);
                        const DST color = colorConvert<SRC,DST>(src[(y + i) * sstride + x]);
                        c |= color << shift;
                    }
                    *d++ = c;
                }
            }
        }

        if (unoptimizedY) {
            const int starty = h - unoptimizedY;
            for (int x = startx; x >= stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride + starty;
                for (int y = starty; y < h; ++y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }
    }
}

template <class SRC, class DST>
static inline void blit90_tiled_unpacked(QScreen *screen, const QImage &image,
                                         const QRect &rect,
                                         const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int numTilesX = (w + tileSize - 1) / tileSize;
    const int numTilesY = (h + tileSize - 1) / tileSize;

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = w - tx * tileSize - 1;
        const int stopx = qMax(startx - tileSize, 0);

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = ty * tileSize;
            const int stopy = qMin(starty + tileSize, h);

            for (int x = startx; x >= stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride + starty;
                for (int y = starty; y < stopy; ++y)
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
            }
        }
    }
}

template <class SRC, class DST>
static inline void blit270_tiled(QScreen *screen, const QImage &image,
                                 const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned =
        qMin((quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST), uint(h));
    const int restX = w % tileSize;
    const int restY = (h - unaligned) % tileSize;
    const int unoptimizedY = restY % pack;
    const int numTilesX = w / tileSize + (restX > 0);
    const int numTilesY = (h - unaligned) / tileSize + (restY >= pack);

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = tx * tileSize;
        const int stopx = qMin(startx + tileSize, w);

        if (unaligned) {
            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride;
                for (int y = h - 1; y >= h - unaligned; --y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = h - 1 - unaligned - ty * tileSize;
            const int stopy = qMax(starty - tileSize, unoptimizedY);

            for (int x = startx; x < stopx; ++x) {
                quint32 *d = reinterpret_cast<quint32*>(dest + x * dstride
                                                        + h - 1 - starty);
                for (int y = starty; y > stopy; y -= pack) {
                    quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
                    for (int i = 1; i < pack; ++i) {
                        const int shift = (sizeof(int) * 8 / pack * i);
                        const DST color = colorConvert<SRC,DST>(src[(y - i) * sstride + x]);
                        c |= color << shift;
                    }
                    *d++ = c;
                }
            }
        }
        if (unoptimizedY) {
            const int starty = unoptimizedY - 1;
            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride + h - 1 - starty;
                for (int y = starty; y >= 0; --y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }
    }
}

template <class SRC, class DST>
static inline void blit270_tiled_unpacked(QScreen *screen, const QImage &image,
                                          const QRect &rect,
                                          const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int numTilesX = (w + tileSize - 1) / tileSize;
    const int numTilesY = (h + tileSize - 1) / tileSize;

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = tx * tileSize;
        const int stopx = qMin(startx + tileSize, w);

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = h - 1 - ty * tileSize;
            const int stopy = qMax(starty - tileSize, 0);

            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride + h - 1 - starty;
                for (int y = starty; y >= stopy; --y)
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
            }
        }
    }
}

#endif // QT_ROTATION_ALFORITHM

template <class SRC, class DST>
static void blit90(QScreen *screen, const QImage &image,
                   const QRect &rect, const QPoint &topLeft)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    blit90_cachedRead<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    blit90_cachedWrite<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
    blit90_packing<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    blit90_tiled<SRC,DST>(screen, image, rect, topLeft);
#endif
}

template <class SRC, class DST>
static void blit90_unpacked(QScreen *screen, const QImage &image,
                            const QRect &rect, const QPoint &topLeft)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    blit90_cachedRead<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    blit90_cachedWrite<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
#warning Packing algorithm not implemented for this depth.
    blit90_cachedRead<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    blit90_tiled_unpacked<SRC,DST>(screen, image, rect, topLeft);
#endif
}

template <class SRC, class DST>
static void blit180(QScreen *screen, const QImage &image,
                    const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                      + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    src += (h - 1) * sstride;
    for (int y = h - 1; y >= 0; --y) {
        for (int x = w - 1; x >= 0; --x) {
            dest[(h - y - 1) * dstride + w - x - 1] = colorConvert<SRC,DST>(src[x]);
        }
        src -= sstride;
    }
}

template <class SRC, class DST>
static void blit270(QScreen *screen, const QImage &image,
                    const QRect &rect, const QPoint &topLeft)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    blit270_cachedRead<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    blit270_cachedWrite<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
#warning Packing algorithm not implemented for this depth.
    blit270_packing<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    blit270_tiled_unpacked<SRC,DST>(screen, image, rect, topLeft);
#endif
}

template <class SRC, class DST>
static void blit270_unpacked(QScreen *screen, const QImage &image,
                             const QRect &rect, const QPoint &topLeft)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    blit270_cachedRead<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    blit270_cachedWrite<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
#warning Packing algorithm not implemented for this depth.
    blit270_cachedRead<SRC,DST>(screen, image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    blit270_tiled_unpacked<SRC,DST>(screen, image, rect, topLeft);
#endif
}

#define SET_BLIT_FUNC(src, dst, rotation, func) \
do {                                            \
    switch (rotation) {                         \
    case Rot90:                                 \
        func = blit90<src, dst>;                \
        break;                                  \
    case Rot180:                                \
        func = blit180<src, dst>;               \
        break;                                  \
    case Rot270:                                \
        func = blit270<src, dst>;               \
        break;                                  \
    default:                                    \
        break;                                  \
    }                                           \
} while (0)

#define SET_BLIT_FUNC_UNPACKED(src, dst, rotation, func)       \
do {                                                           \
    switch (rotation) {                                        \
    case Rot90:                                                \
        func = blit90_unpacked<src, dst>;                      \
        break;                                                 \
    case Rot180:                                               \
        func = blit180<src, dst>;                              \
        break;                                                 \
    case Rot270:                                               \
        func = blit270_unpacked<src, dst>;                     \
        break;                                                 \
    default:                                                   \
        break;                                                 \
    }                                                          \
} while (0)

/*!
    \reimp
*/
void QTransformedScreen::blit(const QImage &image, const QPoint &topLeft,
                              const QRegion &region)
{
    const Transformation trans = d_ptr->transformation;
    if (trans == None) {
        d_ptr->subscreen->blit(image, topLeft, region);
        return;
    }

    const QVector<QRect> rects = region.rects();
    const QRect bound = QRect(0, 0, QScreen::w, QScreen::h)
                        & QRect(topLeft, image.size());

    BlitFunc func = 0;
    switch (depth()) {
#ifdef QT_QWS_DEPTH_32
    case 32:
#ifdef QT_QWS_DEPTH_16
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint16, quint32, trans, func);
        else
#endif
            SET_BLIT_FUNC(quint32, quint32, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_24
    case 24:
        SET_BLIT_FUNC_UNPACKED(quint32, quint24, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_18
    case 18:
        SET_BLIT_FUNC_UNPACKED(quint32, quint18, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_16
    case 16:
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint16, quint16, trans, func);
        else
            SET_BLIT_FUNC(quint32, quint16, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_8
    case 8:
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint16, quint8, trans, func);
        else
            SET_BLIT_FUNC(quint32, quint8, trans, func);
        break;
#endif
    default:
        return;
    }
    if (!func)
        return;

    QWSDisplay::grab();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i) & bound;

        QPoint dst;
        switch (trans) {
        case Rot90:
            dst = mapToDevice(r.topRight(), QSize(w, h));
            break;
        case Rot180:
            dst = mapToDevice(r.bottomRight(), QSize(w, h));
            break;
        case Rot270:
            dst = mapToDevice(r.bottomLeft(), QSize(w, h));
            break;
        default:
            break;
        }
        func(this, image, r.translated(-topLeft), dst);
    }
    QWSDisplay::ungrab();

}

/*!
    \reimp
*/
void QTransformedScreen::solidFill(const QColor &color, const QRegion &region)
{
    QRegion tr = mapToDevice(region, QSize(w,h));

    Q_ASSERT(tr.boundingRect() == mapToDevice(region.boundingRect(), QSize(w,h)));

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::solidFill region" << region << "transformed" << tr;
#endif
    d_ptr->subscreen->solidFill(color, tr);
}

/*!
    \reimp
*/
QSize QTransformedScreen::mapToDevice(const QSize &s) const
{
    switch (d_ptr->transformation) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
    return s;
}

/*!
    \reimp
*/
QSize QTransformedScreen::mapFromDevice(const QSize &s) const
{
    switch (d_ptr->transformation) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
        return s;
}

/*!
    \reimp
*/
QPoint QTransformedScreen::mapToDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    }

    return rp;
}

/*!
    \reimp
*/
QPoint QTransformedScreen::mapFromDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    }

    return rp;
}

/*!
    \reimp
*/
QRect QTransformedScreen::mapToDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (d_ptr->transformation) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    }

    return correctNormalized(tr);
}

/*!
    \reimp
*/
QRect QTransformedScreen::mapFromDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (d_ptr->transformation) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    }

    return correctNormalized(tr);
}

/*!
    \reimp
*/
QRegion QTransformedScreen::mapToDevice(const QRegion &rgn, const QSize &s) const
{
    if (d_ptr->transformation == None)
        return rgn;

#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice size" << s << "rgn:  " << rgn;
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice trgn:  " << trgn;
#endif
    return trgn;
}

/*!
    \reimp
*/
QRegion QTransformedScreen::mapFromDevice(const QRegion &rgn, const QSize &s) const
{
    if (d_ptr->transformation == None)
        return rgn;
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: realRegion count:  " << rgn.rects().size() << " isEmpty? " << rgn.isEmpty() << "  bounds:" << rgn.boundingRect();
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (d_ptr->transformation) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: transRegion count: " << trgn.rects().size() << " isEmpty? " << trgn.isEmpty() << "  bounds:" << trgn.boundingRect();
#endif
    return trgn;
}

/*!
    \reimp
*/
void QTransformedScreen::disconnect()
{
    if (d_ptr->subscreen) {
        d_ptr->subscreen->disconnect();
        delete d_ptr->subscreen;
        d_ptr->subscreen = 0;
    }
}

/*!
    \reimp
*/
bool QTransformedScreen::initDevice()
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->initDevice();

    return false;
}

/*!
    \reimp
*/
void QTransformedScreen::shutdownDevice()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->shutdownDevice();
}

/*!
    \reimp
*/
void QTransformedScreen::setMode(int w,int h, int d)
{
    if (d_ptr->subscreen) {
        d_ptr->subscreen->setMode(w, h, d);
        d_ptr->configure();
        exposeRegion(region(), 0);
    }
}

/*!
    \reimp
*/
bool QTransformedScreen::supportsDepth(int depth) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->supportsDepth(depth);
    return false;
}

/*!
    \reimp
*/
void QTransformedScreen::save()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->save();
    QScreen::save();
}

/*!
    \reimp
*/
void QTransformedScreen::restore()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->restore();
    QScreen::restore();
}

/*!
    \reimp
*/
void QTransformedScreen::blank(bool on)
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->blank(on);
}

/*!
    \reimp
*/
bool QTransformedScreen::onCard(const unsigned char *ptr) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->onCard(ptr);
    return false;
}

/*!
    \reimp
*/
bool QTransformedScreen::onCard(const unsigned char *ptr, ulong &offset) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->onCard(ptr, offset);
    return false;
}

/*!
    \reimp
*/
bool QTransformedScreen::isInterlaced() const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->isInterlaced();
    return false;
}

/*!
    \reimp
*/
int QTransformedScreen::memoryNeeded(const QString &str)
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->memoryNeeded(str);
    else
        return QScreen::memoryNeeded(str);
}

/*!
    \reimp
*/
int QTransformedScreen::sharedRamSize(void *ptr)
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->sharedRamSize(ptr);
    else
        return QScreen::sharedRamSize(ptr);
}

/*!
    \reimp
*/
void QTransformedScreen::haltUpdates()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->haltUpdates();
}

/*!
    \reimp
*/
void QTransformedScreen::resumeUpdates()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->resumeUpdates();
}

/*!
    \reimp
*/
void QTransformedScreen::setDirty(const QRect& rect)
{
    if (!d_ptr->subscreen)
        return;

    const QRect r = mapToDevice(rect, QSize(width(), height()));
    d_ptr->subscreen->setDirty(r);
}

/*!
    \reimp
*/
QWSWindowSurface* QTransformedScreen::createSurface(QWidget *widget) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->createSurface(widget);
    return QScreen::createSurface(widget);
}

/*!
    \reimp
*/
QList<QScreen*> QTransformedScreen::subScreens() const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->subScreens();
    return QScreen::subScreens();
}

/*!
    \reimp
*/
QRegion QTransformedScreen::region() const
{
    QRegion deviceRegion;
    if (d_ptr->subscreen)
        deviceRegion = d_ptr->subscreen->region();
    else
        deviceRegion = QScreen::region();

    const QSize size(deviceWidth(), deviceHeight());
    return mapFromDevice(deviceRegion, size);
}

#endif // QT_NO_QWS_TRANSFORMED
