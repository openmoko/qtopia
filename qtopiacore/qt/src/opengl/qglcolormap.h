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

#ifndef QGLCOLORMAP_H
#define QGLCOLORMAP_H

#include <QtGui/qcolor.h>
#include <QtCore/qvector.h>

QT_BEGIN_HEADER

QT_MODULE(OpenGL)

class Q_OPENGL_EXPORT QGLColormap
{
public:
    QGLColormap();
    QGLColormap(const QGLColormap &);
    ~QGLColormap();

    QGLColormap &operator=(const QGLColormap &);

    bool   isEmpty() const;
    int    size() const;
    void   detach();

    void   setEntries(int count, const QRgb * colors, int base = 0);
    void   setEntry(int idx, QRgb color);
    void   setEntry(int idx, const QColor & color);
    QRgb   entryRgb(int idx) const;
    QColor entryColor(int idx) const;
    int    find(QRgb color) const;
    int    findNearest(QRgb color) const;

protected:
    Qt::HANDLE handle() { return d ? d->cmapHandle : 0; }
    void setHandle(Qt::HANDLE ahandle) { d->cmapHandle = ahandle; }

private:
    struct QGLColormapData {
        QBasicAtomic ref;
        QVector<QRgb> *cells;
        Qt::HANDLE cmapHandle;
    };

    QGLColormapData *d;
    static struct QGLColormapData shared_null;
    static void cleanup(QGLColormapData *x);
    void detach_helper();

    friend class QGLWidget;
    friend class QGLWidgetPrivate;
};

inline void QGLColormap::detach()
{
    if (d->ref != 1)
        detach_helper();
}

QT_END_HEADER

#endif // QGLCOLORMAP_H
