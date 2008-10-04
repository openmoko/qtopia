/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
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
** http://www.gnu.org/copyleft/gpl.html.
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

#include <QtGui/QPaintDevice>
#include <QtGui/QWidget>
#include <QtOpenGL/QGLWidget>
#include "private/qglwindowsurface_qws_p.h"
#include "private/qglpaintdevice_qws_p.h"
#include "private/qpaintengine_opengl_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWSGLWindowSurface
    \since 4.3
    \ingroup qws
    \preliminary

    \brief The QWSGLWindowSurface class provides the drawing area for top-level
    windows with Qt for Embedded Linux on EGL/OpenGL ES. It also provides the
    drawing area for \l{QGLWidget}s whether they are top-level windows or
    children of another QWidget.

    Note that this class is only available in Qt for Embedded Linux and only
    available if Qt is configured with OpenGL support.
*/

class QWSGLWindowSurfacePrivate
{
public:
    QWSGLWindowSurfacePrivate() :
        qglContext(0), ownsContext(false) {}

    QGLContext *qglContext;
    bool ownsContext;
};

/*!
    Constructs an empty QWSGLWindowSurface for the given top-level \a window.
    The window surface is later initialized from chooseContext() and resources for it
    is typically allocated in setGeometry().
*/
QWSGLWindowSurface::QWSGLWindowSurface(QWidget *window)
    : QWSWindowSurface(window),
      d_ptr(new QWSGLWindowSurfacePrivate)
{
}

/*!
    Constructs an empty QWSGLWindowSurface.
*/
QWSGLWindowSurface::QWSGLWindowSurface()
    : d_ptr(new QWSGLWindowSurfacePrivate)
{
}

/*!
    Destroys the QWSGLWindowSurface object and frees any
    allocated resources.
 */
QWSGLWindowSurface::~QWSGLWindowSurface()
{
    Q_D(QWSGLWindowSurface);
    if (d->ownsContext)
        delete d->qglContext;
    delete d;
}

/*!
    Returns the QGLContext of the window surface.
*/
QGLContext *QWSGLWindowSurface::context() const
{
    Q_D(const QWSGLWindowSurface);
    if (!d->qglContext) {
        QWSGLWindowSurface *that = const_cast<QWSGLWindowSurface*>(this);
        that->setContext(new QGLContext(QGLFormat::defaultFormat()));
        that->d_func()->ownsContext = true;
    }
    return d->qglContext;
}

/*!
    Sets the QGLContext for this window surface to \a context.
*/
void QWSGLWindowSurface::setContext(QGLContext *context)
{
    Q_D(QWSGLWindowSurface);
    if (d->ownsContext) {
        delete d->qglContext;
        d->ownsContext = false;
    }
    d->qglContext = context;
}

QT_END_NAMESPACE
