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

#ifndef QGLPIXELBUFFER_P_H
#define QGLPIXELBUFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#if defined (Q_OS_HPUX) && !defined (GLX_VERSION_1_3)
typedef unsigned long GLXPbuffer;

struct GLXFBConfig {
    int visualType;
    int transparentType;
                                /*    colors are floats scaled to ints */
    int transparentRed, transparentGreen, transparentBlue, transparentAlpha;
    int transparentIndex;

    int visualCaveat;

    int associatedVisualId;
    int screen;

    int drawableType;
    int renderType;

    int maxPbufferWidth, maxPbufferHeight, maxPbufferPixels;
    int optimalPbufferWidth, optimalPbufferHeight;  /* for SGIX_pbuffer */

    int visualSelectGroup;	/* visuals grouped by select priority */

    unsigned int id;

    GLboolean rgbMode;
    GLboolean colorIndexMode;
    GLboolean doubleBufferMode;
    GLboolean stereoMode;
    GLboolean haveAccumBuffer;
    GLboolean haveDepthBuffer;
    GLboolean haveStencilBuffer;

    /* The number of bits present in various buffers */
    GLint accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
    GLint depthBits;
    GLint stencilBits;
    GLint indexBits;
    GLint redBits, greenBits, blueBits, alphaBits;
    GLuint redMask, greenMask, blueMask, alphaMask;

    GLuint multiSampleSize;     /* Number of samples per pixel (0 if no ms) */

    GLuint nMultiSampleBuffers; /* Number of available ms buffers */
    GLint maxAuxBuffers;

    /* frame buffer level */
    GLint level;

    /* color ranges (for SGI_color_range) */
    GLboolean extendedRange;
    GLdouble minRed, maxRed;
    GLdouble minGreen, maxGreen;
    GLdouble minBlue, maxBlue;
    GLdouble minAlpha, maxAlpha;
};

#endif

#include "QtOpenGL/qglpixelbuffer.h"
#include <private/qgl_p.h>

#ifdef Q_WS_X11
#include <GL/glx.h>
#elif defined(Q_WS_WIN)
DECLARE_HANDLE(HPBUFFERARB);
#elif defined(Q_WS_MACX)
#include <AGL/agl.h>
#elif defined(Q_WS_QWS)
#include <GLES/egl.h>
#endif

class QGLPixelBufferPrivate {
    Q_DECLARE_PUBLIC(QGLPixelBuffer)
public:
    QGLPixelBufferPrivate() : invalid(true), qctx(0), pbuf(0), ctx(0)
    {
        QGLExtensions::init();
#ifdef Q_WS_WIN
        dc = 0;
#elif defined(Q_WS_MACX)
        share_ctx = 0;
#endif
    }
    bool init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget);
    void common_init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget);
    bool cleanup();

    bool invalid;
    QGLContext *qctx;
    QGLPixelBuffer *q_ptr;
    QGLFormat format;

    QGLFormat req_format;
    QPointer<QGLWidget> req_shareWidget;
    QSize req_size;

#ifdef Q_WS_X11
    GLXPbuffer pbuf;
    GLXContext ctx;
#elif defined(Q_WS_WIN)
    HDC dc;
    HPBUFFERARB pbuf;
    HGLRC ctx;
#elif defined(Q_WS_MACX)
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    AGLPbuffer pbuf;
#else
    void *pbuf;
#endif
    AGLContext ctx;
    AGLContext share_ctx;
#elif defined(Q_WS_QWS)
    void *pbuf;
    EGLContext ctx;
#endif
};

#endif // QGLPIXELBUFFER_P_H
