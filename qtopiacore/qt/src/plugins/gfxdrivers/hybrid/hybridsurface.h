/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef HYBRIDSURFACE_H
#define HYBRIDSURFACE_H

#include <private/qglwindowsurface_qws_p.h>
#include <private/qglpaintdevice_qws_p.h>
#include <GLES/egl.h>
#include <vanilla/eglVanilla.h>

class HybridPaintDevice;
class HybridSurfacePrivate;
class QWSLock;

class HybridSurface : public QWSGLWindowSurface
{
public:
    HybridSurface();
    HybridSurface(QWidget *w, EGLDisplay display);
    ~HybridSurface();

    void beginPaint(const QRegion &region);
    bool lock(int timeout);
    void unlock();

    bool isValid() const;
    void setGeometry(const QRect &rect, const QRegion &mask);
    QString key() const { return QLatin1String("hybrid"); }

    QByteArray permanentState() const;
    void setPermanentState(const QByteArray &state);

    QImage image() const;
    QPaintDevice *paintDevice();
    QPoint painterOffset() const;

private:
    QSharedMemory mem;
    QImage img;
    QWSLock *memlock;
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
    QWSGLPaintDevice *pdevice;

    VanillaPixmap vanillaPix;
};

#endif // HYBRIDSURFACE_H
