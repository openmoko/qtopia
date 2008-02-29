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

#include "qglpixelbuffer.h"
#include "qglpixelbuffer_p.h"
#include <GLES/egl.h>

#include <qimage.h>
#include <private/qgl_p.h>
#include <qdebug.h>

#ifndef GL_TEXTURE_RECTANGLE_EXT
#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#endif

static int nearest_gl_texture_size(int v)
{
    int n = 0, last = 0;
    for (int s = 0; s < 32; ++s) {
        if (((v>>s) & 1) == 1) {
            ++n;
            last = s;
        }
    }
    if (n > 1)
        return 1 << (last+1);
    return 1 << last;
}

bool QGLPixelBufferPrivate::init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
{
    Q_UNUSED(size);
    Q_UNUSED(f);
    Q_UNUSED(shareWidget);
    return false;
}

bool QGLPixelBufferPrivate::cleanup()
{
    return false;
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)
{
    Q_UNUSED(texture_id);
    return false;
}

void QGLPixelBuffer::releaseFromDynamicTexture()
{
}

bool QGLPixelBuffer::makeCurrent()
{
    return false;
}

bool QGLPixelBuffer::doneCurrent()
{
    return false;
}

GLuint QGLPixelBuffer::generateDynamicTexture() const
{
    return 0;
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
    return false;
}
