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

#include "qdirectfbscreen.h"
#include "qdirectfbsurface.h"
#include "qdirectfbpixmap.h"
#include "qdirectfbmouse.h"
#include "qdirectfbkeyboard.h"
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/qcolor.h>
#include <QtGui/qapplication.h>
#include <QtGui/qwindowsystem_qws.h>
#include <QtGui/private/qpixmapdatafactory_p.h>
#include <QtGui/private/qwssignalhandler_p.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvector.h>
#include <QtCore/qrect.h>

class QDirectFBScreenPrivate : public QObject, public QPixmapDataFactory
{
public:
    QDirectFBScreenPrivate();
    ~QDirectFBScreenPrivate();

    void setFlipFlags(const QStringList &args);
    QPixmapData* create(QPixmapData::PixelType type);

    IDirectFB *dfb;
    IDirectFBSurface *dfbSurface;
    DFBSurfaceFlipFlags flipFlags;
#ifndef QT_NO_DIRECTFB_LAYER
    IDirectFBDisplayLayer *dfbLayer;
#endif
    IDirectFBScreen *dfbScreen;
    QRegion prevExpose;

#ifndef QT_NO_DIRECTFB_MOUSE
    QDirectFBMouseHandler *mouse;
#endif
#ifndef QT_NO_DIRECTFB_KEYBOARD
    QDirectFBKeyboardHandler *keyboard;
#endif
    QPixmapDataFactory *bitmapFactory;
};

QDirectFBScreenPrivate::QDirectFBScreenPrivate()
    : dfb(0), dfbSurface(0), flipFlags(DSFLIP_BLIT)
#ifndef QT_NO_DIRECTFB_LAYER
    , dfbLayer(0)
#endif
#ifndef QT_NO_DIRECTFB_MOUSE
    , mouse(0)
#endif
#ifndef QT_NO_DIRECTFB_KEYBOARD
    , keyboard(0)
#endif
    , bitmapFactory(0)
{
#ifndef QT_NO_QWS_SIGNALHANDLER
    QWSSignalHandler::instance()->addObject(this);
#endif
}

QDirectFBScreenPrivate::~QDirectFBScreenPrivate()
{
#ifndef QT_NO_DIRECTFB_MOUSE
    delete mouse;
#endif
#ifndef QT_NO_DIRECTFB_KEYBOARD
    delete keyboard;
#endif
    if (dfbSurface)
        dfbSurface->Release(dfbSurface);
#ifndef QT_NO_DIRECTFB_LAYER
    if (dfbLayer)
        dfbLayer->Release(dfbLayer);
#endif
    if (dfbScreen)
        dfbScreen->Release(dfbScreen);
    if (dfb)
        dfb->Release(dfb);
}

IDirectFB* QDirectFBScreen::dfb()
{
    return d_ptr->dfb;
}

IDirectFBSurface* QDirectFBScreen::dfbSurface()
{
    return d_ptr->dfbSurface;
}

#ifndef QT_NO_DIRECTFB_LAYER
IDirectFBDisplayLayer* QDirectFBScreen::dfbDisplayLayer()
{
    return d_ptr->dfbLayer;
}
#endif

DFBSurfacePixelFormat QDirectFBScreen::getSurfacePixelFormat(const QImage &image)
{
    switch (image.format()) {
#ifndef QT_NO_DIRECTFB_PALETTE
    case QImage::Format_Indexed8:
        return DSPF_LUT8;
#endif
    case QImage::Format_RGB888:
        return DSPF_RGB24;
    case QImage::Format_ARGB4444_Premultiplied:
        return DSPF_ARGB4444;
#if (Q_DIRECTFB_VERSION >= 0x010100)
    case QImage::Format_RGB444:
        return DSPF_RGB444;
    case QImage::Format_RGB555:
        return DSPF_RGB555;
#endif
    case QImage::Format_RGB16:
        return DSPF_RGB16;
    case QImage::Format_ARGB6666_Premultiplied:
        return DSPF_ARGB6666;
    case QImage::Format_RGB666:
        return DSPF_RGB18;
    case QImage::Format_RGB32:
        return DSPF_RGB32;
    case QImage::Format_ARGB32_Premultiplied:
//    case QImage::Format_ARGB32:
        return DSPF_ARGB;
    default:
        return DSPF_UNKNOWN;
    };
}

QImage::Format QDirectFBScreen::getImageFormat(DFBSurfacePixelFormat format)
{
    switch (format) {
    case DSPF_LUT8:
        return QImage::Format_Indexed8;
    case DSPF_RGB24:
        return QImage::Format_RGB888;
    case DSPF_ARGB4444:
        return QImage::Format_ARGB4444_Premultiplied;
#if (Q_DIRECTFB_VERSION >= 0x010100)
    case DSPF_RGB444:
        return QImage::Format_RGB444;
    case DSPF_RGB555:
#endif
    case DSPF_ARGB1555:
        return QImage::Format_RGB555;
    case DSPF_RGB16:
        return QImage::Format_RGB16;
    case DSPF_ARGB6666:
        return QImage::Format_ARGB6666_Premultiplied;
    case DSPF_RGB18:
        return QImage::Format_RGB666;
    case DSPF_RGB32:
        return QImage::Format_RGB32;
    case DSPF_ARGB:
        return QImage::Format_ARGB32_Premultiplied;
    default:
        break;
    }
    return QImage::Format_Invalid;
}

DFBSurfaceDescription QDirectFBScreen::getSurfaceDescription(const QImage &image)
{
    DFBSurfaceDescription description;
    DFBSurfacePixelFormat format = getSurfacePixelFormat(image);

    if (format == DSPF_UNKNOWN || image.isNull()) {
        description.flags = DFBSurfaceDescriptionFlags(0);
        return description;
    }

    description.flags = DFBSurfaceDescriptionFlags(DSDESC_CAPS
                                                   | DSDESC_WIDTH
                                                   | DSDESC_HEIGHT
                                                   | DSDESC_PIXELFORMAT
                                                   | DSDESC_PREALLOCATED);

    description.caps = DSCAPS_NONE;
    description.width = image.width();
    description.height = image.height();
    description.pixelformat = format;
    description.preallocated[0].data = (void*)(image.bits());
    description.preallocated[0].pitch = image.bytesPerLine();
    description.preallocated[1].data = 0;
    description.preallocated[1].pitch = 0;

    switch (image.format()) {
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB4444_Premultiplied:
        description.caps = DSCAPS_PREMULTIPLIED;
    default:
        break;
    }

    return description;
}

DFBSurfaceDescription QDirectFBScreen::getSurfaceDescription(const uint *buffer,
                                                             int length)
{
    DFBSurfaceDescription description;

    description.flags = DFBSurfaceDescriptionFlags(DSDESC_CAPS
                                                   | DSDESC_WIDTH
                                                   | DSDESC_HEIGHT
                                                   | DSDESC_PIXELFORMAT
                                                   | DSDESC_PREALLOCATED);
    description.caps = DSCAPS_PREMULTIPLIED;
    description.width = length;
    description.height = 1;
    description.pixelformat = DSPF_ARGB;
    description.preallocated[0].data = (void*)buffer;
    description.preallocated[0].pitch = length * sizeof(uint);
    description.preallocated[1].data = 0;
    description.preallocated[1].pitch = 0;

    return description;
}

#ifndef QT_NO_DIRECTFB_PALETTE
void QDirectFBScreen::setSurfaceColorTable(IDirectFBSurface *surface,
                                           const QImage &image)
{
    if (!surface)
        return;

    const int numColors = image.numColors();
    if (numColors == 0)
        return;

    QVarLengthArray<DFBColor> colors(numColors);
    for (int i = 0; i < numColors; ++i) {
        QRgb c = image.color(i);
        colors[i].a = qAlpha(c);
        colors[i].r = qRed(c);
        colors[i].g = qGreen(c);
        colors[i].b = qBlue(c);
    }

    IDirectFBPalette *palette;
    DFBResult result;
    result = surface->GetPalette(surface, &palette);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen::setSurfaceColorTable GetPalette",
                      result);
        return;
    }
    result = palette->SetEntries(palette, colors.data(), numColors, 0);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen::setSurfaceColorTable SetEntries",
                      result);
    }
}
#endif // QT_NO_DIRECTFB_PALETTE

#if !defined(QT_NO_DIRECTFB_LAYER) && !defined(QT_NO_QWS_CURSOR)
class Q_GUI_EXPORT QDirectFBScreenCursor : public QScreenCursor
{
public:
    QDirectFBScreenCursor();
    ~QDirectFBScreenCursor();

    void set(const QImage &image, int hotx, int hoty);
    void move(int x, int y);
    void show();
    void hide();

private:
    IDirectFBDisplayLayer *layer;
    bool implicitHide;
};

QDirectFBScreenCursor::QDirectFBScreenCursor()
{
    IDirectFB *fb = QDirectFBScreen::instance()->dfb();
    if (!fb)
        qFatal("QDirectFBScreenCursor: DirectFB not initialized");

    layer = QDirectFBScreen::instance()->dfbDisplayLayer();

    if (layer)
        layer->SetCooperativeLevel(layer, DLSCL_SHARED); // XXX: hw: remove?
    else
        qFatal("QDirectFBScreenCursor: Unable to get primary display layer!");

    enable = true;
    hwaccel = true;
    supportsAlpha = true;

    set(QImage(), 0, 0);
}

QDirectFBScreenCursor::~QDirectFBScreenCursor()
{
}

void QDirectFBScreenCursor::show()
{
    DFBResult result;
    result = layer->SetCooperativeLevel(layer, DLSCL_ADMINISTRATIVE);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::show: "
                      "Unable to set cooperative level", result);
    }
    result = layer->EnableCursor(layer, 1);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::show: "
                      "Unable to enable cursor", result);
    }
    result = layer->SetCooperativeLevel(layer, DLSCL_SHARED);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::show: "
                      "Unable to reset cooperative level", result);
    }
    implicitHide = false;
}

void QDirectFBScreenCursor::hide()
{
    DFBResult result;
    result = layer->SetCooperativeLevel(layer, DLSCL_ADMINISTRATIVE);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::hide: "
                      "Unable to set cooperative level", result);
    }
    result = layer->EnableCursor(layer, 0);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::hide: "
                      "Unable to disable cursor", result);
    }
    result = layer->SetCooperativeLevel(layer, DLSCL_SHARED);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::hide: "
                      "Unable to reset cooperative level", result);
    }
    implicitHide = false;
}

void QDirectFBScreenCursor::move(int x, int y)
{
    layer->WarpCursor(layer, x, y);
}

void QDirectFBScreenCursor::set(const QImage &image, int hotx, int hoty)
{
    if (image.isNull() && isVisible()) {
        hide();
        implicitHide = true;
    } else if (!image.isNull() && implicitHide) {
        show();
    }

#ifdef QT_NO_DIRECTFB_PALETTE
    if (image.numColors() > 0)
        cursor = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    else
#endif
        cursor = image;
    size = cursor.size();
    hotspot = QPoint(hotx, hoty);

    IDirectFB *fb = QDirectFBScreen::instance()->dfb();

    DFBSurfaceDescription description;
    description = QDirectFBScreen::getSurfaceDescription(cursor);

    IDirectFBSurface *surface;
    DFBResult result = fb->CreateSurface(fb, &description, &surface);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::set: Unable to create surface",
                      result);
        return;
    }
#ifndef QT_NO_DIRECTFB_PALETTE
    QDirectFBScreen::setSurfaceColorTable(surface, cursor);
#endif

    result = layer->SetCooperativeLevel(layer, DLSCL_ADMINISTRATIVE);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::set: "
                      "Unable to set cooperative level", result);
    }
    result = layer->SetCursorShape(layer, surface, hotx, hoty);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::set: Unable to set cursor shape",
                      result);
    }

    result = layer->SetCooperativeLevel(layer, DLSCL_SHARED);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreenCursor::set: "
                      "Unable to reset cooperative level", result);
    }

    if (surface)
        surface->Release(surface);
}
#endif // QT_NO_DIRECTFB_LAYER

QDirectFBScreen::QDirectFBScreen(int display_id)
    : QScreen(display_id, DirectFBClass), d_ptr(new QDirectFBScreenPrivate)
{
}

QDirectFBScreen::~QDirectFBScreen()
{
    delete d_ptr;
}

int QDirectFBScreen::depth(DFBSurfacePixelFormat format)
{
    switch (format) {
    case DSPF_A1:
        return 1;
    case DSPF_A8:
    case DSPF_RGB332:
    case DSPF_LUT8:
    case DSPF_ALUT44:
        return 8;
    case DSPF_I420:
    case DSPF_YV12:
    case DSPF_NV12:
    case DSPF_NV21:
#if (Q_DIRECTFB_VERSION >= 0x010100)
    case DSPF_RGB444:
#endif
        return 12;
#if (Q_DIRECTFB_VERSION >= 0x010100)
    case DSPF_RGB555:
        return 15;
#endif
    case DSPF_ARGB1555:
    case DSPF_RGB16:
    case DSPF_YUY2:
    case DSPF_UYVY:
    case DSPF_NV16:
    case DSPF_ARGB2554:
    case DSPF_ARGB4444:
        return 16;
    case DSPF_RGB24:
        return 24;
    case DSPF_RGB32:
    case DSPF_ARGB:
    case DSPF_AiRGB:
        return 32;
    case DSPF_UNKNOWN:
    default:
        return 0;
    };
    return 0;
}

void QDirectFBScreenPrivate::setFlipFlags(const QStringList &args)
{
    QRegExp flipRegexp(QLatin1String("^flip=([\\w,]+)$"));
    int index = args.indexOf(flipRegexp);
    if (index >= 0) {
        const QStringList flips = flipRegexp.cap(1).split(QLatin1Char(','),
                                                          QString::SkipEmptyParts);
        flipFlags = DSFLIP_NONE;
        foreach (QString flip, flips) {
            if (flip == QLatin1String("wait"))
                flipFlags = DFBSurfaceFlipFlags(flipFlags | DSFLIP_WAIT);
            else if (flip == QLatin1String("blit"))
                flipFlags = DFBSurfaceFlipFlags(flipFlags | DSFLIP_BLIT);
            else if (flip == QLatin1String("onsync"))
                flipFlags = DFBSurfaceFlipFlags(flipFlags | DSFLIP_ONSYNC);
            else if (flip == QLatin1String("pipeline"))
                flipFlags = DFBSurfaceFlipFlags(flipFlags | DSFLIP_PIPELINE);
            else
                qWarning("QDirectFBScreen: Unknown flip argument: %s",
                         qPrintable(flip));
        }
    }
}

QPixmapData* QDirectFBScreenPrivate::create(QPixmapData::PixelType type)
{
    if (type == QPixmapData::BitmapType && bitmapFactory)
        return bitmapFactory->create(type);

    return new QDirectFBPixmapData(type);
}

static void printDirectFBInfo(IDirectFB *fb)
{
    DFBResult result;
    DFBGraphicsDeviceDescription dev;

    result = fb->GetDeviceDescription(fb, &dev);
    if (result != DFB_OK) {
        DirectFBError("Error reading graphics device description", result);
        return;
    }

    qDebug("Device: %s (%s), Driver: %s v%i.%i (%s)\n"
           "  acceleration: 0x%x, blit: 0x%x, draw: 0x%0x video: %i\n",
           dev.name, dev.vendor, dev.driver.name, dev.driver.major,
           dev.driver.minor, dev.driver.vendor, dev.acceleration_mask,
           dev.blitting_flags, dev.drawing_flags, dev.video_memory);
}

bool QDirectFBScreen::connect(const QString &displaySpec)
{
    DFBResult result = DFB_OK;

    {   // pass command line arguments to DirectFB
        const QStringList args = QCoreApplication::arguments();
        int argc = args.size();
        char **argv = new char*[argc];

        for (int i = 0; i < argc; ++i)
            argv[i] = qstrdup(args.at(i).toLocal8Bit().constData());

        result = DirectFBInit(&argc, &argv);
        if (result != DFB_OK) {
            DirectFBError("QDirectFBScreen: error initializing DirectFB",
                          result);
        }
        delete[] argv;
    }

    const QStringList args = displaySpec.split(QLatin1Char(':'),
                                               QString::SkipEmptyParts);

    d_ptr->setFlipFlags(args);

    result = DirectFBCreate(&d_ptr->dfb);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen: error creating DirectFB interface",
                      result);
        return false;
    }

    if (args.contains(QLatin1String("debug"), Qt::CaseInsensitive))
        printDirectFBInfo(d_ptr->dfb);

#ifndef QT_NO_DIRECTFB_WM
    if (args.contains(QLatin1String("fullscreen")))
#endif
        d_ptr->dfb->SetCooperativeLevel(d_ptr->dfb, DFSCL_FULLSCREEN);

    DFBSurfaceDescription description;
    description.flags = DFBSurfaceDescriptionFlags(DSDESC_CAPS);
    description.caps = DFBSurfaceCapabilities(DSCAPS_PRIMARY
                                              | DSCAPS_DOUBLE
                                              | DSCAPS_STATIC_ALLOC);
    if (d_ptr->flipFlags ^ DSFLIP_BLIT) {
        description.caps = DFBSurfaceCapabilities(description.caps
                                                  | DSCAPS_FLIPPING);
    }
    result = d_ptr->dfb->CreateSurface(d_ptr->dfb, &description,
                                       &d_ptr->dfbSurface);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen: error creating primary surface",
                      result);
        return false;
    }
    d_ptr->dfbSurface->GetSize(d_ptr->dfbSurface, &w, &h);

    data = 0;
    lstep = 0;
    size = 0;
    dw = w;
    dh = h;

    DFBSurfacePixelFormat format;
    result = d_ptr->dfbSurface->GetPixelFormat(d_ptr->dfbSurface, &format);
    if (result == DFB_OK)
        QScreen::d = depth(format);
    else
        DirectFBError("QDirectFBScreen: error getting surface format", result);

    setPixelFormat(getImageFormat(format));

    const int dpi = 72;
    physWidth = qRound(dw * 25.4 / dpi);
    physHeight = qRound(dh * 25.4 / dpi);

#ifndef QT_NO_DIRECTFB_LAYER
    result = d_ptr->dfb->GetDisplayLayer(d_ptr->dfb, DLID_PRIMARY,
                                         &d_ptr->dfbLayer);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen::connect: "
                      "Unable to get primary display layer!", result);
        return false;
    }
    result = d_ptr->dfbLayer->GetScreen(d_ptr->dfbLayer, &d_ptr->dfbScreen);
#else
    result = d_ptr->dfb->GetScreen(d_ptr->dfb, 0, &d_ptr->dfbScreen);
#endif
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen::connect: "
                      "Unable to get screen!", result);
        return false;
    }

    d_ptr->bitmapFactory = QScreen::pixmapDataFactory();
    setPixmapDataFactory(d_ptr);

    return true;
}

void QDirectFBScreen::disconnect()
{
    d_ptr->dfbSurface->Release(d_ptr->dfbSurface);
    d_ptr->dfbSurface = 0;
#ifndef QT_NO_DIRECTFB_LAYER
    d_ptr->dfbLayer->Release(d_ptr->dfbLayer);
    d_ptr->dfbLayer = 0;
#endif
    d_ptr->dfb->Release(d_ptr->dfb);
    d_ptr->dfb = 0;
}

bool QDirectFBScreen::initDevice()
{
    QWSServer *server = QWSServer::instance();
#ifndef QT_NO_DIRECTFB_MOUSE
    if (qgetenv("QWS_MOUSE_PROTO").isEmpty()) {
        server->setDefaultMouse("None");
        d_ptr->mouse = new QDirectFBMouseHandler;
    }
#endif
#ifndef QT_NO_DIRECTFB_KEYBOARD
    if (qgetenv("QWS_KEYBOARD").isEmpty()) {
        server->setDefaultKeyboard("None");
        d_ptr->keyboard = new QDirectFBKeyboardHandler(QString());
    }
#endif

#ifndef QT_NO_QWS_CURSOR
#ifdef QT_NO_DIRECTFB_LAYER
    QScreenCursor::initSoftwareCursor();
#else
    qt_screencursor = new QDirectFBScreenCursor;
#endif
#endif
    return true;
}

void QDirectFBScreen::shutdownDevice()
{
#ifndef QT_NO_DIRECTFB_MOUSE
    delete d_ptr->mouse;
    d_ptr->mouse = 0;
#endif
#ifndef QT_NO_DIRECTFB_KEYBOARD
    delete d_ptr->keyboard;
    d_ptr->keyboard = 0;
#endif

#ifndef QT_NO_QWS_CURSOR
    delete qt_screencursor;
    qt_screencursor = 0;
#endif
}

void QDirectFBScreen::setMode(int width, int height, int depth)
{
    d_ptr->dfb->SetVideoMode(d_ptr->dfb, width, height, depth);
}

void QDirectFBScreen::blank(bool on)
{
    d_ptr->dfbScreen->SetPowerMode(d_ptr->dfbScreen,
                                   (on ? DSPM_ON : DSPM_SUSPEND));
}

QWSWindowSurface* QDirectFBScreen::createSurface(QWidget *widget) const
{
#ifdef QT_NO_DIRECTFB_WM
    if (QApplication::type() == QApplication::GuiServer)
        return new QDirectFBSurface(widget);
    else
        return QScreen::createSurface(widget);
#else
    return new QDirectFBSurface(widget);
#endif
}

QWSWindowSurface* QDirectFBScreen::createSurface(const QString &key) const
{
    if (key == QLatin1String("directfb"))
        return new QDirectFBSurface;
    return QScreen::createSurface(key);
}

void QDirectFBScreen::compose(const QRegion &region)
{
    const QList<QWSWindow*> windows = QWSServer::instance()->clientWindows();

    QRegion blitRegion = region;
    QRegion blendRegion;

    d_ptr->dfbSurface->SetBlittingFlags(d_ptr->dfbSurface, DSBLIT_NOFX);

    // blit opaque region
    for (int i = 0; i < windows.size(); ++i) {
        QWSWindow *win = windows.at(i);
        QWSWindowSurface *surface = win->windowSurface();
        if (!surface)
            continue;

        const QRegion r = win->allocatedRegion() & blitRegion;
        if (r.isEmpty())
            continue;

        blitRegion -= r;

        if (surface->isRegionReserved()) {
            // nothing
        } else if (win->isOpaque()) {
            const QPoint offset = win->requestedRegion().boundingRect().topLeft();

            if (surface->key() == QLatin1String("directfb")) {
                QDirectFBSurface *s = static_cast<QDirectFBSurface*>(surface);
                blit(s->surface(), offset, r);
            } else {
                blit(surface->image(), offset, r);
            }
        } else {
            blendRegion += r;
        }
        if (blitRegion.isEmpty())
            break;
    }

    { // fill background
        const QRegion fill = blitRegion + blendRegion;
        if (!fill.isEmpty()) {
            const QColor color = QWSServer::instance()->backgroundBrush().color();
            solidFill(color, fill);
            blitRegion = QRegion();
        }
    }

    if (blendRegion.isEmpty())
        return;

    // blend non-opaque region
    for (int i = windows.size() - 1; i >= 0; --i) {
        QWSWindow *win = windows.at(i);
        QWSWindowSurface *surface = win->windowSurface();
        if (!surface)
            continue;

        const QRegion r = win->allocatedRegion() & blendRegion;
        if (r.isEmpty())
            continue;

        DFBSurfaceBlittingFlags flags = DSBLIT_NOFX;
        if (!win->isOpaque()) {
            flags = DFBSurfaceBlittingFlags(flags | DSBLIT_BLEND_ALPHACHANNEL);
            const uint opacity = win->opacity();
            if (opacity < 255) {
                flags = DFBSurfaceBlittingFlags(flags | DSBLIT_BLEND_COLORALPHA);
                d_ptr->dfbSurface->SetColor(d_ptr->dfbSurface, 0xff, 0xff, 0xff, opacity);
            }
        }
        d_ptr->dfbSurface->SetBlittingFlags(d_ptr->dfbSurface, flags);

        const QPoint offset = win->requestedRegion().boundingRect().topLeft();

        if (surface->key() == QLatin1String("directfb")) {
            QDirectFBSurface *s = static_cast<QDirectFBSurface*>(surface);
            blit(s->surface(), offset, r);
        } else {
            blit(surface->image(), offset, r);
        }
    }
}

void QDirectFBScreen::exposeRegion(QRegion r, int changing)
{
    const QList<QWSWindow*> windows = QWSServer::instance()->clientWindows();
    if (changing < 0 || changing >= windows.size())
        return;

#ifndef QT_NO_DIRECTFB_WM
    QWSWindow *win = windows.at(changing);
    QWSWindowSurface *s = win->windowSurface();
    if (s && s->key() == QLatin1String("directfb"))
        return;
#endif

    r &= region();
    if (r.isEmpty())
        return;

    if (d_ptr->flipFlags & DSFLIP_BLIT) {
        const QRect brect = r.boundingRect();
        DFBRegion dfbRegion = { brect.left(), brect.top(),
                                brect.right(), brect.bottom() };
        compose(r);
        d_ptr->dfbSurface->Flip(d_ptr->dfbSurface, &dfbRegion,
                                d_ptr->flipFlags);
    } else {
        compose(r + d_ptr->prevExpose);
        d_ptr->dfbSurface->Flip(d_ptr->dfbSurface, 0, d_ptr->flipFlags);
    }
    d_ptr->prevExpose = r;
}


void QDirectFBScreen::blit(const QImage &img, const QPoint &topLeft,
                           const QRegion &reg)
{
    IDirectFBSurface *src = 0;
    DFBSurfaceDescription description = getSurfaceDescription(img);

    DFBResult result = d_ptr->dfb->CreateSurface(d_ptr->dfb, &description,
                                                 &src);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen::blit(): Error creating surface",
                      result);
        return;
    }
#ifndef QT_NO_DIRECTFB_PALETTE
    setSurfaceColorTable(d_ptr->dfbSurface, img);
#endif

    blit(src, topLeft, reg);

    src->Release(src);
}

void QDirectFBScreen::blit(IDirectFBSurface *src, const QPoint &topLeft,
                           const QRegion &region)
{
    const QVector<QRect> rs = region.translated(-offset()).rects();
    const int size = rs.size();
    const QPoint tl = topLeft - offset();

    QVarLengthArray<DFBRectangle> rects(size);
    QVarLengthArray<DFBPoint> points(size);

    int n = 0;
    for (int i = 0; i < size; ++i) {
        const QRect r = rs.at(i);
        if (!r.isValid())
            continue;
        rects[n].x = r.x() - tl.x();
        rects[n].y = r.y() - tl.y();
        rects[n].w = r.width();
        rects[n].h = r.height();
        points[n].x = r.x();
        points[n].y = r.y();
        ++n;
    }

    d_ptr->dfbSurface->BatchBlit(d_ptr->dfbSurface, src, rects.data(),
                                 points.data(), n);
}

void QDirectFBScreen::solidFill(const QColor &color, const QRegion &region)
{
    if (region.isEmpty())
        return;

    const QVector<QRect> rects = region.rects();
    QVarLengthArray<DFBRectangle> dfbRects(rects.size());
    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        dfbRects[i].x = r.x();
        dfbRects[i].y = r.y();
        dfbRects[i].w = r.width();
        dfbRects[i].h = r.height();
    }

    d_ptr->dfbSurface->SetColor(d_ptr->dfbSurface,
                                color.red(), color.green(), color.blue(),
                                color.alpha());
    d_ptr->dfbSurface->FillRectangles(d_ptr->dfbSurface, dfbRects.data(),
                                      dfbRects.size());
}

