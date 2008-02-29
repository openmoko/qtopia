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
