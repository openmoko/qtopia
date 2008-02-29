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

#ifndef QWINDOWSURFACE_QWS_P_H
#define QWINDOWSURFACE_QWS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qwindowsurface_p.h"
#include <qregion.h>
#include <qimage.h>
#include <private/qsharedmemory_p.h>

class QScreen;
class QWSWindowSurfacePrivate;

class Q_GUI_EXPORT QWSWindowSurface : public QWindowSurface
{
public:
    QWSWindowSurface();
    QWSWindowSurface(QWidget *widget);
    ~QWSWindowSurface();

    virtual void release();

    virtual bool isValidFor(const QWidget *widget) const = 0;

    virtual void setGeometry(const QRect &rect);
    virtual QRect geometry() const = 0;

    virtual void scroll(const QRegion &region, int dx, int dy) = 0;

    virtual void flush(QWidget *widget, const QRegion &region,
                       const QPoint &offset);

    virtual QPoint painterOffset() const;

    virtual void beginPaint(const QRegion &) {}
    virtual void endPaint(const QRegion &) {}

    virtual const QString key() const = 0;
    virtual const QByteArray data() const = 0;

    virtual bool attach(const QByteArray &data) = 0;
    virtual void detach() = 0;

    virtual const QImage image() const = 0;
    virtual QPaintDevice *paintDevice() = 0;

    QWidget *window() const;

    const QRegion dirtyRegion() const;
    void setDirty(const QRegion &) const;

    const QRegion clipRegion() const;
    void setClipRegion(const QRegion &);

    enum SurfaceFlag {
        Reserved = 0x1,
        Buffered = 0x2,
        Opaque = 0x4
    };
    Q_DECLARE_FLAGS(SurfaceFlags, SurfaceFlag)

    SurfaceFlags surfaceFlags() const;

    inline bool isReserved() const { return surfaceFlags() & Reserved; }
    inline bool isBuffered() const { return surfaceFlags() & Buffered; }
    inline bool isOpaque() const { return surfaceFlags() & Opaque; }

protected:
    void setSurfaceFlags(SurfaceFlags type);

private:
    QWSWindowSurfacePrivate *d_ptr;
};

class QWSLock;

class Q_GUI_EXPORT QWSMemorySurface : public QWSWindowSurface
{
public:
    QWSMemorySurface();
    QWSMemorySurface(QWidget *widget);
    ~QWSMemorySurface();

    void release();
    void detach();

    void beginPaint(const QRegion &);
    void endPaint(const QRegion &);

    bool isValidFor(const QWidget *widget) const;

    QPaintDevice *paintDevice() { return &img; }
    QRect geometry() const;
    void scroll(const QRegion &area, int dx, int dy);

    const QImage image() const { return img; };
    QPoint painterOffset() const;

protected:
    QImage::Format preferredImageFormat(const QWidget *widget) const;
    void setLock(int lockId);

    QWSLock *memlock;
    QImage img;
};

class Q_GUI_EXPORT QWSLocalMemSurface : public QWSMemorySurface
{
public:
    QWSLocalMemSurface();
    QWSLocalMemSurface(QWidget *widget);
    ~QWSLocalMemSurface() {}

    void setGeometry(const QRect &rect);

    const QString key() const { return QLatin1String("mem"); }
    const QByteArray data() const;

    bool attach(const QByteArray &data);
    void detach();
    void release();

protected:
    uchar *mem;
    int memsize;
};

#ifndef QT_NO_QWS_MULTIPROCESS
class Q_GUI_EXPORT QWSSharedMemSurface : public QWSMemorySurface
{
public:
    QWSSharedMemSurface();
    QWSSharedMemSurface(QWidget *widget);
    ~QWSSharedMemSurface();

    void setGeometry(const QRect &rect);

    const QString key() const { return QLatin1String("shm"); }
    const QByteArray data() const;

    bool attach(const QByteArray &data);
    void detach();
    void release();

private:
    bool setMemory(int memId);

    QSharedMemory mem;
};
#endif // QT_NO_QWS_MULTIPROCESS

class Q_GUI_EXPORT QWSOnScreenSurface : public QWSMemorySurface
{
public:
    QWSOnScreenSurface();
    QWSOnScreenSurface(QWidget *widget);
    ~QWSOnScreenSurface();

    bool isValidFor(const QWidget *widget) const;
    QPoint painterOffset() const;

    void setGeometry(const QRect &rect);
    QRect geometry() const { return brect; }

    const QString key() const { return QLatin1String("OnScreen"); }
    const QByteArray data() const;

    bool attach(const QByteArray &data);
    void detach() {}

private:
    void attachToScreen(const QScreen *screen);

    mutable QRect brect;
    const QScreen *screen;
};

class Q_GUI_EXPORT QWSYellowSurface : public QWSWindowSurface
{
public:
    QWSYellowSurface(bool isClient = false);
    ~QWSYellowSurface();

    void setDelay(int msec) { delay = msec; }

    void setGeometry(const QRect &rect);
    QRect geometry() const;

    void scroll(const QRegion &, int, int) {}
    bool isValidFor(const QWidget *) const { return true; }

    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

    const QString key() const { return QLatin1String("Yellow"); }
    const QByteArray data() const;

    bool attach(const QByteArray &data);
    void detach();

    QPaintDevice *paintDevice() { return &img; }
    const QImage image() const { return img; }

private:
    int winId;
    int delay;
    QSize surfaceSize; // client side
    QImage img; // server side
};

#ifndef QT_NO_DIRECTPAINTER

class QScreen;

class Q_GUI_EXPORT QWSDirectPainterSurface : public QWSWindowSurface
{
public:
    QWSDirectPainterSurface(bool isClient = false);
    ~QWSDirectPainterSurface();

    void setReserved() { setSurfaceFlags(Reserved); }
    void release();

    void setGeometry(const QRect &rect) { setRegion(rect); }
    QRect geometry() const { return clipRegion().boundingRect(); }

    void setRegion(const QRegion &region);
    QRegion region() const { return clipRegion(); }

    void flush(QWidget*, const QRegion &, const QPoint &) {};
    void scroll(const QRegion &, int, int) {};

    bool isValidFor(const QWidget*) const { return false; }

    const QString key() const { return QLatin1String("DirectPainter"); }
    const QByteArray data() const;

    bool attach(const QByteArray &);
    void detach() {}

    const QImage image() const { return QImage(); }
    QPaintDevice *paintDevice() { return 0; }

    WId windowId() const { return static_cast<WId>(winId); }

    QScreen *screen() const { return _screen; }
private:
    int winId;
    QScreen *_screen;
};

#endif // QT_NO_DIRECTPAINTER

#endif // QWINDOWSURFACE_QWS_P_H
