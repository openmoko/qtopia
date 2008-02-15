/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
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
#include <qlibrary.h>
#include <qdebug.h>
#include <private/qgl_p.h>
#include <private/qt_x11_p.h>
#include <private/qpaintengine_opengl_p.h>

#include <qx11info_x11.h>
#include <GL/glx.h>
#include <qimage.h>

#include "qglpixelbuffer.h"
#include "qglpixelbuffer_p.h"

#ifndef GLX_VERSION_1_3
#define GLX_RGBA_BIT            0x00000002
#define GLX_PBUFFER_BIT         0x00000004
#define GLX_DRAWABLE_TYPE       0x8010
#define GLX_RENDER_TYPE         0x8011
#define GLX_RGBA_TYPE           0x8014
#define GLX_PBUFFER_HEIGHT      0x8040
#define GLX_PBUFFER_WIDTH       0x8041
#endif

#ifndef GLX_ARB_multisample
#define GLX_SAMPLE_BUFFERS_ARB  100000
#define GLX_SAMPLES_ARB         100001
#endif

typedef GLXFBConfig* (*_glXChooseFBConfig) (Display *dpy, int screen, const int *attrib_list, int *nelements);
typedef int (*_glXGetFBConfigAttrib) (Display *dpy, GLXFBConfig config, int attribute, int *value);
typedef GLXPbuffer (*_glXCreatePbuffer) (Display *dpy, GLXFBConfig config, const int *attrib_list);
typedef void (*_glXDestroyPbuffer) (Display *dpy, GLXPbuffer pbuf);
typedef GLXContext (*_glXCreateNewContext) (Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef Bool (*_glXMakeContextCurrent) (Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);

static _glXChooseFBConfig qt_glXChooseFBConfig = 0;
static _glXCreateNewContext qt_glXCreateNewContext = 0;
static _glXCreatePbuffer qt_glXCreatePbuffer = 0;
static _glXDestroyPbuffer qt_glXDestroyPbuffer = 0;
static _glXGetFBConfigAttrib qt_glXGetFBConfigAttrib = 0;
static _glXMakeContextCurrent qt_glXMakeContextCurrent = 0;

#define glXChooseFBConfig qt_glXChooseFBConfig
#define glXCreateNewContext qt_glXCreateNewContext
#define glXCreatePbuffer qt_glXCreatePbuffer
#define glXDestroyPbuffer qt_glXDestroyPbuffer
#define glXGetFBConfigAttrib qt_glXGetFBConfigAttrib
#define glXMakeContextCurrent qt_glXMakeContextCurrent

static bool qt_resolve_pbuffer_extensions()
{
    static int resolved = false;
    if (resolved && qt_glXMakeContextCurrent)
        return true;
    else if (resolved)
        return false;

    extern const QString qt_gl_library_name();
    QLibrary gl(qt_gl_library_name());
    qt_glXChooseFBConfig = (_glXChooseFBConfig) gl.resolve("glXChooseFBConfig");
    qt_glXCreateNewContext = (_glXCreateNewContext) gl.resolve("glXCreateNewContext");
    qt_glXCreatePbuffer = (_glXCreatePbuffer) gl.resolve("glXCreatePbuffer");
    qt_glXDestroyPbuffer = (_glXDestroyPbuffer) gl.resolve("glXDestroyPbuffer");
    qt_glXGetFBConfigAttrib = (_glXGetFBConfigAttrib) gl.resolve("glXGetFBConfigAttrib");
    qt_glXMakeContextCurrent = (_glXMakeContextCurrent) gl.resolve("glXMakeContextCurrent");

    resolved = qt_glXMakeContextCurrent ? true : false;
    return resolved;
}

static void qt_format_to_attrib_list(const QGLFormat &f, int attribs[])
{
    int i = 0;
    attribs[i++] = GLX_RENDER_TYPE;
    attribs[i++] = GLX_RGBA_BIT;
    attribs[i++] = GLX_DRAWABLE_TYPE;
    attribs[i++] = GLX_PBUFFER_BIT;
    attribs[i++] = GLX_RED_SIZE;
    attribs[i++] = f.redBufferSize() == -1 ? 1 : f.redBufferSize();
    attribs[i++] = GLX_GREEN_SIZE;
    attribs[i++] = f.greenBufferSize() == -1 ? 1 : f.greenBufferSize();
    attribs[i++] = GLX_BLUE_SIZE;
    attribs[i++] = f.blueBufferSize() == -1 ? 1 : f.blueBufferSize();
    if (f.doubleBuffer()) {
        attribs[i++] = GLX_DOUBLEBUFFER;
        attribs[i++] = true;
    }
    if (f.depth()) {
        attribs[i++] = GLX_DEPTH_SIZE;
        attribs[i++] = f.depthBufferSize() == -1 ? 1 : f.depthBufferSize();
    }
    if (f.stereo()) {
        attribs[i++] = GLX_STEREO;
        attribs[i++] = true;
    }
    if (f.stencil()) {
        attribs[i++] = GLX_STENCIL_SIZE;
        attribs[i++] = f.stencilBufferSize() == -1 ? 1 : f.stencilBufferSize();
    }
    if (f.alpha()) {
        attribs[i++] = GLX_ALPHA_SIZE;
        attribs[i++] = f.alphaBufferSize() == -1 ? 1 : f.alphaBufferSize();
    }
    if (f.accum()) {
        attribs[i++] = GLX_ACCUM_RED_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        attribs[i++] = GLX_ACCUM_GREEN_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        attribs[i++] = GLX_ACCUM_BLUE_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        if (f.alpha()) {
            attribs[i++] = GLX_ACCUM_ALPHA_SIZE;
            attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        }
    }
    if (f.sampleBuffers()) {
        attribs[i++] = GLX_SAMPLE_BUFFERS_ARB;
        attribs[i++] = 1;
        attribs[i++] = GLX_SAMPLES_ARB;
        attribs[i++] = f.samples() == -1 ? 4 : f.samples();
    }

    attribs[i] = XNone;
}

bool QGLPixelBufferPrivate::init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
{
    if (!qt_resolve_pbuffer_extensions()) {
        qWarning("QGLPixelBuffer: pbuffers are not supported on this system.");
        return false;
    }

    int attribs[40];
    int num_configs = 0;

    qt_format_to_attrib_list(f, attribs);

    GLXFBConfig *configs = glXChooseFBConfig(X11->display, X11->defaultScreen, attribs, &num_configs);
    if (configs && num_configs) {
        int res;
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_LEVEL, &res);
        format.setPlane(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_DOUBLEBUFFER, &res);
        format.setDoubleBuffer(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_DEPTH_SIZE, &res);
        format.setDepth(res);
        if (format.depth())
            format.setDepthBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_RGBA, &res);
        format.setRgba(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_RED_SIZE, &res);
        format.setRedBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_GREEN_SIZE, &res);
        format.setGreenBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_BLUE_SIZE, &res);
        format.setBlueBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_ALPHA_SIZE, &res);
        format.setAlpha(res);
        if (format.alpha())
            format.setAlphaBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_ACCUM_RED_SIZE, &res);
        format.setAccum(res);
        if (format.accum())
            format.setAccumBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_STENCIL_SIZE, &res);
        format.setStencil(res);
        if (format.stencil())
            format.setStencilBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_STEREO, &res);
        format.setStereo(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_SAMPLE_BUFFERS_ARB, &res);
        format.setSampleBuffers(res);
        if (format.sampleBuffers()) {
            glXGetFBConfigAttrib(X11->display, configs[0], GLX_SAMPLES_ARB, &res);
            format.setSamples(res);
        }

        int pb_attribs[] = {GLX_PBUFFER_WIDTH, size.width(), GLX_PBUFFER_HEIGHT, size.height(), XNone};
        GLXContext shareContext = 0;
        if (shareWidget && shareWidget->d_func()->glcx)
            shareContext = (GLXContext) shareWidget->d_func()->glcx->d_func()->cx;

        pbuf = glXCreatePbuffer(QX11Info::display(), configs[0], pb_attribs);
        ctx = glXCreateNewContext(QX11Info::display(), configs[0], GLX_RGBA_TYPE, shareContext, true);

        XFree(configs);
        if (!pbuf || !ctx) {
            qWarning("QGLPixelBuffer: Unable to create a pbuffer/context - giving up.");
            return false;
        }
        return true;
    } else {
        qWarning("QGLPixelBuffer: Unable to find a context/format match - giving up.");
        return false;
    }
}

bool QGLPixelBufferPrivate::cleanup()
{
    glXDestroyPbuffer(QX11Info::display(), pbuf);
    return true;
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint)
{
    return false;
}

void QGLPixelBuffer::releaseFromDynamicTexture()
{
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
    bool ret = qt_resolve_pbuffer_extensions();

    if (!ret)
	return false;

    int attribs[40];
    int num_configs = 0;

    qt_format_to_attrib_list(QGLFormat::defaultFormat(), attribs);

    GLXFBConfig *configs = glXChooseFBConfig(X11->display, X11->defaultScreen, attribs, &num_configs);
    GLXPbuffer pbuf = 0;
    GLXContext ctx = 0;

    if (configs && num_configs) {
        int pb_attribs[] = {GLX_PBUFFER_WIDTH, 128, GLX_PBUFFER_HEIGHT, 128, XNone};
        pbuf = glXCreatePbuffer(X11->display, configs[0], pb_attribs);
        ctx = glXCreateNewContext(X11->display, configs[0], GLX_RGBA_TYPE, 0, true);
        XFree(configs);
	glXDestroyContext(X11->display, ctx);
	glXDestroyPbuffer(X11->display, pbuf);
    }
    return pbuf && ctx;
}
