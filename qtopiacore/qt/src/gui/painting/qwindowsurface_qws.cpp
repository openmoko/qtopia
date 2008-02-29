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

#include "qwindowsurface_qws_p.h"
#include <qwidget.h>
#include <qscreen_qws.h>
#include <qwsmanager_qws.h>
#include <qapplication.h>
#include <qwsdisplay_qws.h>
#include <qrgb.h>
#include <qpaintengine.h>
#include <qdesktopwidget.h>
#include <private/qapplication_p.h>
#include <private/qwsdisplay_qws_p.h>
#include <private/qwidget_p.h>
#include <private/qwsmanager_p.h>
#include <private/qwslock_p.h>
#include <stdio.h>

static inline bool isWidgetOpaque(const QWidget *w)
{
    const QBrush brush = w->palette().brush(w->backgroundRole());
    return (brush.style() == Qt::NoBrush || brush.isOpaque());
}

static inline QScreen *getScreen(const QWidget *w)
{
    const QList<QScreen*> subScreens = qt_screen->subScreens();
    if (subScreens.isEmpty())
        return qt_screen;

    const int screen = QApplication::desktop()->screenNumber(w);

    return qt_screen->subScreens().at(screen < 0 ? 0 : screen);
}


static inline void setImageMetrics(QImage &img, QWidget *window) {
    QScreen *myScreen = getScreen(window);
    if (myScreen) {
        int dpmx = myScreen->width()*1000 / myScreen->physicalWidth();
        int dpmy = myScreen->height()*1000 / myScreen->physicalHeight();
        img.setDotsPerMeterX(dpmx);
        img.setDotsPerMeterY(dpmy);
    }
}


class QWSWindowSurfacePrivate
{
public:
    QWSWindowSurfacePrivate() : widget(0), flags(0) {}

    QWidget *widget;
    QWSWindowSurface::SurfaceFlags flags;
    QRegion dirty;
    QRegion clip;
    QRegion clippedDirty; // dirty, but currently outside the clip region
};

/*!
    \class QWSWindowSurface
    \since 4.2
    \ingroup qws
    \preliminary

    \brief The QWSWindowSurface class provides the drawing area for top-level
    windows in Qtopia Core.

    Note that this class is only available in Qtopia Core.

    In \l {Qtopia Core}, the default behavior is for each client to
    render its widgets into memory while the server is responsible for
    putting the contents of the memory onto the
    screen. QWSWindowSurface is used by the window system to implement
    the associated memory allocation.

    When a screen update is required, the server runs through all the
    top-level windows that intersect with the region that is about to
    be updated, and ensures that the associated clients have updated
    their memory buffer. Then the server uses the screen driver to
    copy the content of the memory to the screen. To locate the
    relevant parts of memory, the driver is provided with the list of
    top-level windows that intersect with the given region. Associated
    with each of the top-level windows there is a window surface
    representing the drawing area of the window.

    When deriving from the QWSWindowSurface class, e.g., when adding
    an \l {Adding an Accelerated Graphics Driver in Qtopia
    Core}{accelerated graphics driver}, there are several pure virtual
    functions that must be implemented. In addition, QWSWindowSurface
    provides several virtual functions that can be reimplemented to
    customize the drawing process.

    \tableofcontents

    \section1 Pure Virtual Functions

    There are in fact two window surface instances for each top-level
    window; one used by the application when drawing a window, and
    another used by the server application to perform window
    compositioning. Implement the attach() to create the server-side
    representation of the surface. The data() function must be
    implemented to provide the required data. In addition, the related
    detach() function must be implemented to enable the window system
    to detach the server-side instance from the client.

    Implement the key() function to uniquely identify the surface
    class, and the isValidFor() function to determine is a surface
    corresponds to a given widget.

    The geometry() function must be implemented to let the window
    system determine the area required by the window surface
    (QWSWindowSurface also provides a corresponding virtual
    setGeometry() function that is called whenever the area necessary
    for the top-level window to be drawn, changes). The image()
    function is called by the window system during window
    compositioning, and must be implemented to return an image of the
    top-level window.

    Finally, the paintDevice() function must be implemented to return
    the appropriate paint device, and the scroll() function must be
    implemented to scroll the given region of the surface the given
    number of pixels.

    \section1 Virtual Functions

    When painting onto the surface, the window system will always call
    the beginPaint() function before any painting operations are
    performed. Likewise the endPaint() function is automatically
    called when the painting is done. Reimplement the painterOffset()
    function to alter the offset that is applied when drawing.

    The window system uses the flush() function to put a given region
    of the widget onto the screen, and the release() function to
    deallocate the screen region corresponding to this window surface.

    \section1 Other Members

    QWSWindowSurface provides the window() function returning a
    pointer to the top-level window the surface is representing. The
    currently visible region of the associated widget can be retrieved
    and set using the clipRegion() and setClipRegion() functions,
    respectively.

    When the window system performs the window compositioning, it uses
    the SurfaceFlag enum describing the surface content. The currently
    set surface flags can be retrieved and altered using the
    surfaceFlags() and setSurfaceFlags() functions. In addition,
    QWSWindowSurface provides the isBuffered(), isOpaque() and
    isReserved() convenience functions.  Use the dirtyRegion()
    function to retrieve the part of the widget that must be
    repainted, and the setDirty() function to ensure that a region is
    repainted.

    \sa {Qtopia Core Architecture#Drawing on Screen}{Qtopia
    Core Architecture}
*/

/*!
    \fn QPaintDevice* QWSWindowSurface::paintDevice()

    Implement this function to return the appropriate paint device.

    \sa painterOffset()
*/

/*!
    \enum QWSWindowSurface::SurfaceFlag

    This enum is used to describe the window surface's contents.  It
    is used by the screen driver to handle region allocation and
    composition.

    \value Reserved The surface contains a reserved area. Once
    allocated, a reserved area can not not be changed by the window
    system, i.e., no other widgets can be drawn on top of this.

    \value Buffered
    The surface is in a memory area which is not part of a framebuffer.
    (A top-level window with QWidget::windowOpacity() other than 1.0 must use
    a buffered surface in order to making blending with the background work.)

    \value Opaque
    The surface contains only opaque pixels.

    \sa surfaceFlags(), setSurfaceFlags()
*/

/*!
    \fn void QWSWindowSurface::scroll(const QRegion &region, int dx, int dy)

    Scrolls the given \a region \a dx pixels to the right and \a dy
    downward; both \a dx and \a dy may be negative.

    \sa geometry()
*/

/*!
    \fn bool QWSWindowSurface::isValidFor(const QWidget *window) const

    Implement this function to return true if the surface is a valid
    surface for the given top-level \a window; otherwise return
    false.

    \sa window(), key()
*/

/*!
    \fn QRect QWSWindowSurface::geometry() const

    Implement this function to return the currently allocated
    area.

    \sa setGeometry(), scroll(), image()
*/

/*!
    \fn void QWSWindowSurface::beginPaint(const QRegion &region)

    This function is called before painting onto the surface begins,
    with the \a region in which the painting will occur.

    \sa endPaint(), paintDevice()
*/

/*!
    \fn void QWSWindowSurface::endPaint(const QRegion &region)

    This function is called after painting onto the surface has ended,
    with the \a region in which the painting was performed.

    \sa beginPaint(), paintDevice()
*/

/*!
    \fn const QString QWSWindowSurface::key() const

    Implement this function to return a string that uniquely
    identifies the class of this surface.

    \sa window(), isValidFor()
*/

/*!
    \fn const QByteArray QWSWindowSurface::data() const

    Implement this function to return the data required for creating a
    server-side representation of the surface.

    \sa attach()
*/

/*!
    \fn bool QWSWindowSurface::attach(const QByteArray &data)

    Implement this function to attach a server-side surface instance
    to the corresponding client side instance using the given \a
    data. Return true if successful; otherwise return false.

    \sa detach(), data()
*/

/*!
    \fn void QWSWindowSurface::detach()

    Implement this function to detach a server-side instance from the
    client.

    \sa attach(), release()
*/

/*!
    \fn const QImage QWSWindowSurface::image() const

    Implement this function to return an image of the top-level window.

    \sa geometry()
*/

/*!
    \fn bool QWSWindowSurface::isReserved() const

    Returns true if the QWSWindowSurface::Reserved is set; otherwise
    returns false.

    \sa surfaceFlags()
*/

/*!
    \fn bool QWSWindowSurface::isBuffered() const

    Returns true if the QWSWindowSurface::Buffered is set; otherwise returns false.

    \sa surfaceFlags()
*/

/*!
    \fn bool QWSWindowSurface::isOpaque() const

    Returns true if the QWSWindowSurface::Opaque is set; otherwise
    returns false.

    \sa surfaceFlags()
*/


/*!
    Constructs an empty surface.
*/
QWSWindowSurface::QWSWindowSurface()
    : d_ptr(new QWSWindowSurfacePrivate)
{
}

/*!
    Constructs an empty surface for the given top-level \a window.
*/
QWSWindowSurface::QWSWindowSurface(QWidget *window)
    : d_ptr(new QWSWindowSurfacePrivate)
{
    d_ptr->widget = window;
}

/*!
    Destroys this surface.
*/
QWSWindowSurface::~QWSWindowSurface()
{
    delete d_ptr;
}

/*!
    Returns the offset to be used when painting.

    \sa paintDevice()
*/
QPoint QWSWindowSurface::painterOffset() const
{
    const QWidget *w = window();
    if (!w)
        return QPoint();
    return w->geometry().topLeft() - w->frameGeometry().topLeft();
}

/*!
    Returns a pointer to the top-level window associated with this
    surface.

    \sa isValidFor(), key()
*/
QWidget *QWSWindowSurface::window() const
{
    return d_ptr->widget;
}

/*!
    Returns the region that must be repainted.

    \sa setDirty()
*/
const QRegion QWSWindowSurface::dirtyRegion() const
{
    return d_ptr->dirty;
}

/*!
    Marks the given \a region as dirty, i.e., altered.

    \sa dirtyRegion()
*/
void QWSWindowSurface::setDirty(const QRegion &region) const
{
    if (region.isEmpty())
        return;

    QRegion unclipped = region;
    if (!d_ptr->clip.isEmpty()) {
#ifdef QT_EXPERIMENTAL_REGIONS
        if (!qt_region_strictContains(d_ptr->clip, region.boundingRect()))
#endif
        {
            d_ptr->clippedDirty += (region - d_ptr->clip);
            unclipped &= d_ptr->clip;
        }
    }
    d_ptr->dirty += unclipped;

    if (window() && !unclipped.isEmpty())
        QApplication::postEvent(window(), new QEvent(QEvent::UpdateRequest));
}

/*!
    Returns the region currently visible on the screen.

    \sa setClipRegion()
*/
const QRegion QWSWindowSurface::clipRegion() const
{
    return d_ptr->clip;
}

/*!
    Sets the region currently visible on the screen to be the given \a
    clip region.

    \sa clipRegion()
*/
void QWSWindowSurface::setClipRegion(const QRegion &clip)
{
    if (clip == d_ptr->clip)
        return;

    QRegion expose = (clip - d_ptr->clip);
    QRegion dirtyExpose;
    d_ptr->clip = clip;

    if (isBuffered()) {
        dirtyExpose = expose & d_ptr->clippedDirty;
        d_ptr->clippedDirty -= expose;
    }
#ifndef QT_NO_QWS_MANAGER
    if (window() && !expose.isEmpty()) {
        QTLWExtra *topextra = window()->d_func()->extra->topextra;
        QWSManager *manager = topextra->qwsManager;
        if (manager) {
            const QRegion r = manager->region().translated(
                -window()->geometry().topLeft()) & expose;
            if (!r.isEmpty())
                 manager->d_func()->dirtyRegion(QDecoration::All,
                                                QDecoration::Normal, r);
        }
    }
#endif

    if (!window())
        return;
    if (!dirtyExpose.isEmpty()) {
        d_ptr->dirty += expose;
        window()->d_func()->invalidateBuffer(dirtyExpose);
    } else if (!expose.isEmpty()) {
        // XXX: prevent flush() from resetting dirty and from flushing too much
        const QRegion oldDirty = d_ptr->dirty;
        d_ptr->dirty = QRegion();
        flush(window(), expose, QPoint());
        d_ptr->dirty = oldDirty;
    }
}

/*!
    Returns the surface flags describing the contents of this surface.

    \sa isBuffered(), isOpaque(), isReserved()
*/
QWSWindowSurface::SurfaceFlags QWSWindowSurface::surfaceFlags() const
{
    return d_ptr->flags;
}

/*!
    Sets the surface flags describing the contents of this surface, to
    be the given \a flags.

    \sa surfaceFlags()
*/
void QWSWindowSurface::setSurfaceFlags(SurfaceFlags flags)
{
    d_ptr->flags = flags;
}

/*!
    \fn void QWSWindowSurface::setGeometry(const QRect &rectangle)

    Sets the currently allocated area to be the given \a rectangle.

    This function is called whenever area covered by the top-level
    window changes.

    \sa geometry()
*/
void QWSWindowSurface::setGeometry(const QRect &rect)
{
    if (!window())
        return;

    Q_ASSERT(rect == window()->frameGeometry());

    QRegion region = rect;

#ifndef QT_NO_QWS_MANAGER
    QTLWExtra *topextra = window()->d_func()->extra->topextra;
    QWSManager *manager = topextra->qwsManager;

    if (manager) {
        manager->d_func()->dirtyRegion(QDecoration::All, QDecoration::Normal);

        // The frame geometry is the bounding rect of manager->region, which
        // could be too much, so we need to clip.
        region &= (manager->region() + window()->geometry());
    }
#endif

    if (!window()->d_func()->extra->mask.isEmpty())
        region &= window()->d_func()->extra->mask.translated(
            window()->geometry().topLeft());

    QWidget::qwsDisplay()->requestRegion(window()->data->winid,
                                         key(), data(), region);

    setDirty(region.translated(-window()->geometry().topLeft()));
}

static inline void flushUpdate(QWidget *widget, const QRegion &region,
                               const QPoint &offset)
{
    static int delay = -1;

    if (delay == -1)
        delay = qgetenv("QT_FLUSH_UPDATE").toInt() * 10;

    if (delay == 0)
        return;

    static QWSYellowSurface surface(true);
    surface.setDelay(delay);
    surface.flush(widget, region, offset);
}

/*!
    Flushes the given \a region from the specified \a widget onto the
    screen.

    Note that the \a offset parameter is currently unused.

    \sa painterOffset()
*/
void QWSWindowSurface::flush(QWidget *widget, const QRegion &region,
                             const QPoint &offset)
{
    if (!window())
        return;

    Q_UNUSED(offset);

    const bool opaque = isWidgetOpaque(window());
    QRegion toFlush = (region + dirtyRegion());
    if (!d_ptr->clip.isEmpty())
        toFlush &= d_ptr->clip;
    const QRegion stillDirty = (d_ptr->dirty - toFlush);

#ifndef QT_NO_QWS_MANAGER
    QTLWExtra *topextra = window()->d_func()->extra->topextra;
    QWSManager *manager = topextra->qwsManager;
    if (manager)
        manager->d_func()->paint(paintDevice(), toFlush);
#endif

    flushUpdate(widget, toFlush, QPoint(0, 0));

    toFlush.translate(window()->mapToGlobal(QPoint(0, 0)));

    window()->qwsDisplay()->repaintRegion(window()->data->winid, opaque, toFlush);

    d_ptr->dirty = QRegion();
    setDirty(stillDirty);
}

/*!
    Releases the current allocated screen region associated with this
    window surface.

    \sa detach()
*/
void QWSWindowSurface::release()
{
    QWidget::qwsDisplay()->requestRegion(window()->data->winid, key(), data(),
                                         QRegion());
}

static void scroll(const QImage &img, const QRect &rect, const QPoint &point)
{
    uchar *mem = const_cast<uchar*>(img.bits());

    int lineskip = img.bytesPerLine();
    int depth = img.depth() >> 3;

    const QRect r = rect;
    const QPoint p = rect.topLeft() + point;

    const uchar *src;
    uchar *dest;

    if (r.top() < p.y()) {
        src = mem + r.bottom() * lineskip + r.left() * depth;
        dest = mem + (p.y() + r.height() - 1) * lineskip + p.x() * depth;
        lineskip = -lineskip;
    } else {
        src = mem + r.top() * lineskip + r.left() * depth;
        dest = mem + p.y() * lineskip + p.x() * depth;
    }

    const int w = r.width();
    int h = r.height();
    const int bytes = w * depth;
    do {
        ::memmove(dest, src, bytes);
        dest += lineskip;
        src += lineskip;
    } while (--h);
}

static inline void lock(QWSLock *l)
{
    if (l)
        l->lock(QWSLock::BackingStore);
}

static inline void unlock(QWSLock *l)
{
    if (l)
        l->unlock(QWSLock::BackingStore);
}

QWSMemorySurface::QWSMemorySurface()
    : QWSWindowSurface(), memlock(0)
{
    setSurfaceFlags(Buffered);
}

QWSMemorySurface::QWSMemorySurface(QWidget *w)
    : QWSWindowSurface(w)
{
    SurfaceFlags flags = Buffered;
    if (isWidgetOpaque(w))
        flags |= Opaque;
    setSurfaceFlags(flags);

    memlock = QWSDisplay::Data::getClientLock();
}

QWSMemorySurface::~QWSMemorySurface()
{
}

QRect QWSMemorySurface::geometry() const
{
    const QPoint offset = window()->frameGeometry().topLeft();
    const QSize size = img.size();
    return QRect(offset, size);
}

void QWSMemorySurface::release()
{
    QWSWindowSurface::release();
    memlock = 0;
    img = QImage();
}

void QWSMemorySurface::detach()
{
    delete memlock;
    memlock = 0;
    img = QImage();
}

QImage::Format
QWSMemorySurface::preferredImageFormat(const QWidget *widget) const
{
    const bool opaque = isWidgetOpaque(widget);

    if (opaque && getScreen(widget)->depth() <= 16)
        return QImage::Format_RGB16;
    else
        return QImage::Format_ARGB32_Premultiplied;
}

void QWSMemorySurface::setLock(int lockId)
{
    if (memlock && memlock->id() == lockId)
        return;
    delete memlock;
    memlock = (lockId == -1 ? 0 : new QWSLock(lockId));
    return;
}

bool QWSMemorySurface::isValidFor(const QWidget *widget) const
{
    if (img == QImage())
        return true;

    if (preferredImageFormat(widget) != img.format())
        return false;

    if (isOpaque() != isWidgetOpaque(widget))
        return false;

    return true;
}

void QWSMemorySurface::scroll(const QRegion &area, int dx, int dy)
{
    const QVector<QRect> rects = area.rects();
    lock(memlock);
    for (int i = 0; i < rects.size(); ++i)
        ::scroll(img, rects.at(i), QPoint(dx, dy));
    unlock(memlock);
}

void QWSMemorySurface::beginPaint(const QRegion &region)
{
    lock(memlock);
    QWSWindowSurface::beginPaint(region);
}

void QWSMemorySurface::endPaint(const QRegion &region)
{
    QWSWindowSurface::endPaint(region);
    unlock(memlock);
}

QPoint QWSMemorySurface::painterOffset() const
{
    const QWidget *w = window();
    if (!w)
        return QPoint();

    if (w->mask().isEmpty())
        return QWSWindowSurface::painterOffset();

    const QRegion region = w->mask()
                           & w->frameGeometry().translated(-w->geometry().topLeft());
    return -region.boundingRect().topLeft();
}

QWSLocalMemSurface::QWSLocalMemSurface()
    : QWSMemorySurface(), mem(0), memsize(0)
{
}

QWSLocalMemSurface::QWSLocalMemSurface(QWidget *w)
    : QWSMemorySurface(w), mem(0), memsize(0)
{
}

void QWSLocalMemSurface::setGeometry(const QRect &rect)
{
    QSize size = rect.size();

    const QWidget *w = window();
    if (w && !w->mask().isEmpty()) {
        const QRegion region = w->mask()
                               & rect.translated(-w->geometry().topLeft());
        size = region.boundingRect().size();
    }

    if (img.size() != size) {
        QImage::Format imageFormat = preferredImageFormat(window());
        const int bytesPerPixel = imageFormat == QImage::Format_RGB16 ? 2 : 4;

        const int bpl = (size.width() * bytesPerPixel + 3) & ~3;
        memsize = bpl * size.height();

        delete[] mem;
        if (memsize == 0) {
            mem = 0;
            img = QImage();
        } else {
            mem = new uchar[memsize];
            img = QImage(mem, size.width(), size.height(), imageFormat);
            setImageMetrics(img, window());
        }
    }

    QWSWindowSurface::setGeometry(rect);
}

void QWSLocalMemSurface::release()
{
    QWSMemorySurface::release();
    delete[] mem;
    mem = 0;
    memsize = 0;
}

const QByteArray QWSLocalMemSurface::data() const
{
    QByteArray array;
    array.resize(sizeof(uchar*) + 2 * sizeof(int) + sizeof(QImage::Format) +
                 sizeof(SurfaceFlags));

    Q_ASSERT(memlock == 0);

    char *ptr = array.data();

    *reinterpret_cast<uchar**>(ptr) = mem;
    ptr += sizeof(uchar*);

    reinterpret_cast<int*>(ptr)[0] = img.width();
    reinterpret_cast<int*>(ptr)[1] = img.height();
    ptr += 2 * sizeof(int);

    *reinterpret_cast<QImage::Format*>(ptr) = img.format();
    ptr += sizeof(QImage::Format);

    *reinterpret_cast<SurfaceFlags*>(ptr) = surfaceFlags();

    return array;
}

bool QWSLocalMemSurface::attach(const QByteArray &data)
{
    int width;
    int height;
    QImage::Format format;
    SurfaceFlags flags;

    const char *ptr = data.constData();

    mem = *reinterpret_cast<uchar* const*>(ptr);
    ptr += sizeof(uchar*);

    width = reinterpret_cast<const int*>(ptr)[0];
    height = reinterpret_cast<const int*>(ptr)[1];
    ptr += 2 * sizeof(int);

    format = *reinterpret_cast<const QImage::Format*>(ptr);
    ptr += sizeof(QImage::Format);

    flags = *reinterpret_cast<const SurfaceFlags*>(ptr);

    QWSMemorySurface::img = QImage(mem, width, height, format);
    setSurfaceFlags(flags);

    return true;
}

void QWSLocalMemSurface::detach()
{
    QWSMemorySurface::detach();
    delete[] mem;
    mem = 0;
    memsize = 0;
}

#ifndef QT_NO_QWS_MULTIPROCESS

QWSSharedMemSurface::QWSSharedMemSurface()
    : QWSMemorySurface()
{
}

QWSSharedMemSurface::QWSSharedMemSurface(QWidget *widget)
    : QWSMemorySurface(widget)
{
}

QWSSharedMemSurface::~QWSSharedMemSurface()
{
}

bool QWSSharedMemSurface::setMemory(int memId)
{
    if (mem.id() == memId)
        return true;

    mem.detach();
    if (!mem.attach(memId)) {
        perror("QWSSharedMemSurface: attaching to shared memory");
        qCritical("QWSSharedMemSurface: Error attaching to"
                  " shared memory 0x%x", memId);
        return false;
    }

    return true;
}

bool QWSSharedMemSurface::attach(const QByteArray &data)
{
    int memId;
    int width;
    int height;
    int lockId;
    QImage::Format format;
    SurfaceFlags flags;

    const char *ptr = data.constData();

    memId = reinterpret_cast<const int*>(ptr)[0];
    width = reinterpret_cast<const int*>(ptr)[1];
    height = reinterpret_cast<const int*>(ptr)[2];
    lockId = reinterpret_cast<const int*>(ptr)[3];
    ptr += 4 * sizeof(int);

    format = *reinterpret_cast<const QImage::Format*>(ptr);
    ptr += sizeof(QImage::Format);
    flags = *reinterpret_cast<const SurfaceFlags*>(ptr);

    setSurfaceFlags(flags);
    setMemory(memId);
    setLock(lockId);

    uchar *base = static_cast<uchar*>(mem.address());
    QWSMemorySurface::img = QImage(base, width, height, format);

    return true;
}

void QWSSharedMemSurface::detach()
{
    QWSMemorySurface::detach();
    mem.detach();
}

void QWSSharedMemSurface::setGeometry(const QRect &rect)
{
    const QSize size = rect.size();
    if (img.size() != size) {
        QImage::Format imageFormat = preferredImageFormat(window());
        const int bytesPerPixel = imageFormat == QImage::Format_RGB16 ? 2 : 4;

        const int bpl = (size.width() * bytesPerPixel + 3) & ~3;
        const int imagesize = bpl * size.height();

        if (imagesize == 0) {
            mem.detach();
            img = QImage();
        } else {
            mem.detach();
            if (!mem.create(imagesize)) {
                perror("QWSBackingStore::create allocating shared memory");
                qFatal("Error creating shared memory of size %d", imagesize);
            }
            uchar *base = static_cast<uchar*>(mem.address());
            img = QImage(base, size.width(), size.height(), imageFormat);
            setImageMetrics(img, window());
        }
    }

    QWSWindowSurface::setGeometry(rect);
}

void QWSSharedMemSurface::release()
{
    QWSMemorySurface::release();
    mem.detach();
}

const QByteArray QWSSharedMemSurface::data() const
{
    QByteArray array;
    array.resize(4 * sizeof(int) + sizeof(QImage::Format) +
                 sizeof(SurfaceFlags));

    char *ptr = array.data();

    reinterpret_cast<int*>(ptr)[0] = mem.id();
    reinterpret_cast<int*>(ptr)[1] = img.width();
    reinterpret_cast<int*>(ptr)[2] = img.height();
    reinterpret_cast<int*>(ptr)[3] = (memlock ? memlock->id() : -1);
    ptr += 4 * sizeof(int);

    *reinterpret_cast<QImage::Format*>(ptr) = img.format();
    ptr += sizeof(QImage::Format);

    *reinterpret_cast<SurfaceFlags*>(ptr) = surfaceFlags();

    return array;
}

#endif // QT_NO_QWS_MULTIPROCESS

QWSOnScreenSurface::QWSOnScreenSurface(QWidget *w)
    : QWSMemorySurface(w)
{
    attachToScreen(getScreen(w));
    setSurfaceFlags(Opaque);
}

QWSOnScreenSurface::QWSOnScreenSurface()
    : QWSMemorySurface()
{
    setSurfaceFlags(Opaque);
}

void QWSOnScreenSurface::attachToScreen(const QScreen *s)
{
    screen = s;
    uchar *base = screen->base();
    QImage::Format format;
    switch (screen->depth()) {
    case 16:
        format = QImage::Format_RGB16;
        break;
    case 32:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        qFatal("QWSOnScreenSurface::attachToScreen(): screen depth %d "
               "not implemented", screen->depth());
        return;
    }
    QWSMemorySurface::img = QImage(base, screen->width(),
                                   screen->height(), format);
}

QWSOnScreenSurface::~QWSOnScreenSurface()
{
}

QPoint QWSOnScreenSurface::painterOffset() const
{
    const QPoint offset = brect.topLeft() - screen->offset();
    return offset + QWSWindowSurface::painterOffset();
}

bool QWSOnScreenSurface::isValidFor(const QWidget *widget) const
{
    if (screen != getScreen(widget))
        return false;
    if (img.isNull())
        return false;
    if (!isWidgetOpaque(widget))
        return false;
    return true;
}

void QWSOnScreenSurface::setGeometry(const QRect &rect)
{
    brect = rect;
    QWSWindowSurface::setGeometry(rect);
}

const QByteArray QWSOnScreenSurface::data() const
{
    QByteArray array;
    array.resize(sizeof(int));
    int *ptr = reinterpret_cast<int*>(array.data());
    ptr[0] = QApplication::desktop()->screenNumber(window());
    return array;
}

bool QWSOnScreenSurface::attach(const QByteArray &data)
{
    const int *ptr = reinterpret_cast<const int*>(data.constData());
    const int screenNo = ptr[0];

    QScreen *screen = qt_screen;
    if (screenNo > 0)
        screen = qt_screen->subScreens().at(screenNo);
    attachToScreen(screen);

    return true;
}

QWSYellowSurface::QWSYellowSurface(bool isClient)
    : QWSWindowSurface(), delay(10)
{
    if (isClient) {
        winId = QWidget::qwsDisplay()->takeId();
        QWidget::qwsDisplay()->nameRegion(winId,
                                          QLatin1String("Debug flush paint"),
                                          QLatin1String("Silly yellow thing"));
        QWidget::qwsDisplay()->setAltitude(winId, 1, true);
    }
    setSurfaceFlags(Buffered);
}

QWSYellowSurface::~QWSYellowSurface()
{
}

const QByteArray QWSYellowSurface::data() const
{
    QByteArray array;
    array.resize(2 * sizeof(int));

    int *ptr = reinterpret_cast<int*>(array.data());
    ptr[0] = surfaceSize.width();
    ptr[1] = surfaceSize.height();

    return array;
}

bool QWSYellowSurface::attach(const QByteArray &data)
{
    const int *ptr = reinterpret_cast<const int*>(data.constData());

    const int width = ptr[0];
    const int height = ptr[1];

    img = QImage(width, height, QImage::Format_ARGB32);
    img.fill(qRgba(255,255,31,127));

    return true;
}

void QWSYellowSurface::detach()
{
    img = QImage();
}

void QWSYellowSurface::setGeometry(const QRect &)
{
}

QRect QWSYellowSurface::geometry() const
{
    return QRect(QPoint(0, 0), img.size());
}

void QWSYellowSurface::flush(QWidget *widget, const QRegion &region,
                             const QPoint &offset)
{
    Q_UNUSED(offset);

    QWSDisplay *display = QWidget::qwsDisplay();
    QRegion rgn = region;

    if (widget)
        rgn.translate(widget->mapToGlobal(QPoint(0, 0)));

    surfaceSize = region.boundingRect().size();

    display->requestRegion(winId, key(), data(), rgn);
    display->setAltitude(winId, 1, true);
    display->repaintRegion(winId, false, rgn);

    ::usleep(500 * delay);
    display->requestRegion(winId, key(), data(), QRegion());
    ::usleep(500 * delay);
}

#ifndef QT_NO_DIRECTPAINTER

static inline QScreen *getPrimaryScreen()
{
    QScreen *screen = QScreen::instance();
    if (!screen->base()) {
        QList<QScreen*> subScreens = screen->subScreens();
        if (subScreens.size() < 1)
            return 0;
        screen = subScreens.at(0);
    }
    return screen;
}

QWSDirectPainterSurface::~QWSDirectPainterSurface()
{
}

QWSDirectPainterSurface::QWSDirectPainterSurface(bool isClient)
    : QWSWindowSurface((QWidget*)0)
{
    setSurfaceFlags(Opaque);

    if (isClient) {
        winId  = QWidget::qwsDisplay()->takeId();
        QWidget::qwsDisplay()->nameRegion(winId,
                                          QLatin1String("QDirectPainter reserved space"),
                                          QLatin1String("reserved"));
    }
    _screen = QScreen::instance();
    if (!_screen->base()) {
        QList<QScreen*> subScreens = _screen->subScreens();
        if (subScreens.size() < 1)
            _screen = 0;
        else
            _screen = subScreens.at(0);
    }
}

void QWSDirectPainterSurface::setRegion(const QRegion &region)
{
    QRegion reg = region;

    if (_screen->isTransformed()) {
        const QSize devSize(_screen->deviceWidth(), _screen->deviceHeight());
        reg = _screen->mapFromDevice(region, devSize);
    }

    QWidget::qwsDisplay()->requestRegion(winId, key(), data(), reg);

}

void QWSDirectPainterSurface::release()
{
    QWidget::qwsDisplay()->destroyRegion(winId);
}



const QByteArray QWSDirectPainterSurface::data() const
{
    QByteArray res;
    if (isReserved())
        res.append( 'r');
    return res;
}

bool QWSDirectPainterSurface::attach(const QByteArray &ba)
{
    if (ba.size() > 0 && ba.at(0) == 'r')
        setReserved();
    setSurfaceFlags(surfaceFlags() | Opaque);

    return true;
}


#endif // QT_NO_DIRECTPAINTER
