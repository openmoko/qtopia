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

#include <qdebug.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <qglframebufferobject.h>
#include <qlibrary.h>
#include <qimage.h>

// #define DEPTH_BUFFER

#ifndef GL_EXT_framebuffer_object
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                    0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT                            0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT                              0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT                             0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT               0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT               0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT             0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT     0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT        0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT                             0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT                0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT        0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT      0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT                0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                   0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT               0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT               0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                          0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS_EXT                            0x8CDF
#define GL_COLOR_ATTACHMENT0_EXT                                0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT                                0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT                                0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT                                0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT                                0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT                                0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT                                0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT                                0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT                                0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT                                0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT                               0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT                               0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT                               0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT                               0x8CED
#define GL_COLOR_ATTACHMENT14_EXT                               0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT                               0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT                                 0x8D00
#define GL_STENCIL_ATTACHMENT_EXT                               0x8D20
#define GL_FRAMEBUFFER_EXT                                      0x8D40
#define GL_RENDERBUFFER_EXT                                     0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT                               0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT                              0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                     0x8D44
#define GL_STENCIL_INDEX_EXT                                    0x8D45
#define GL_STENCIL_INDEX1_EXT                                   0x8D46
#define GL_STENCIL_INDEX4_EXT                                   0x8D47
#define GL_STENCIL_INDEX8_EXT                                   0x8D48
#define GL_STENCIL_INDEX16_EXT                                  0x8D49
#define GL_RENDERBUFFER_RED_SIZE_EXT                            0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT                          0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT                           0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT                          0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT                          0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT                        0x8D55
#endif

// ### hm. should be part of the GL 1.2 spec..
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE                  0x812F
#endif

#if defined(Q_WS_X11) || defined(Q_WS_MAC) || defined(Q_WS_QWS)

#define QGL_FUNC_CONTEXT

PFNGLISRENDERBUFFEREXTPROC qt_glIsRenderbufferEXT;
PFNGLBINDRENDERBUFFEREXTPROC qt_glBindRenderbufferEXT;
PFNGLDELETERENDERBUFFERSEXTPROC qt_glDeleteRenderbuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC qt_glGenRenderbuffersEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC qt_glRenderbufferStorageEXT;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC qt_glGetRenderbufferParameterivEXT;
PFNGLISFRAMEBUFFEREXTPROC qt_glIsFramebufferEXT;
PFNGLBINDFRAMEBUFFEREXTPROC qt_glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC qt_glDeleteFramebuffersEXT;
PFNGLGENFRAMEBUFFERSEXTPROC qt_glGenFramebuffersEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC qt_glCheckFramebufferStatusEXT;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC qt_glFramebufferTexture1DEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC qt_glFramebufferTexture2DEXT;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC qt_glFramebufferTexture3DEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC qt_glFramebufferRenderbufferEXT;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC qt_glGetFramebufferAttachmentParameterivEXT;
PFNGLGENERATEMIPMAPEXTPROC qt_glGenerateMipmapEXT;

#define glIsRenderbufferEXT qt_glIsRenderbufferEXT
#define glBindRenderbufferEXT qt_glBindRenderbufferEXT
#define glDeleteRenderbuffersEXT qt_glDeleteRenderbuffersEXT
#define glGenRenderbuffersEXT qt_glGenRenderbuffersEXT
#define glRenderbufferStorageEXT qt_glRenderbufferStorageEXT
#define glGetRenderbufferParameterivEXT qt_glGetRenderbufferParameterivEXT
#define glIsFramebufferEXT qt_glIsFramebufferEXT
#define glBindFramebufferEXT qt_glBindFramebufferEXT
#define glDeleteFramebuffersEXT qt_glDeleteFramebuffersEXT
#define glGenFramebuffersEXT qt_glGenFramebuffersEXT
#define glCheckFramebufferStatusEXT qt_glCheckFramebufferStatusEXT
#define glFramebufferTexture1DEXT qt_glFramebufferTexture1DEXT
#define glFramebufferTexture2DEXT qt_glFramebufferTexture2DEXT
#define glFramebufferTexture3DEXT qt_glFramebufferTexture3DEXT
#define glFramebufferRenderbufferEXT qt_glFramebufferRenderbufferEXT
#define glGetFramebufferAttachmentParameterivEXT qt_glGetFramebufferAttachmentParameterivEXT
#define glGenerateMipmapEXT qt_glGenerateMipmapEXT

static bool qt_resolve_framebufferobject_extensions(QGLContext *)
{
    static bool resolved = false;
    if (resolved && qt_glIsRenderbufferEXT)
        return true;
    else if (resolved)
        return false;

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
    QLibrary lib(QLatin1String("GL"));
#else // Q_WS_MAC
    QLibrary lib(QLatin1String("/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"));
#endif

    qt_glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC) lib.resolve("glIsRenderbufferEXT");
    qt_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) lib.resolve("glBindRenderbufferEXT");
    qt_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) lib.resolve("glDeleteRenderbuffersEXT");
    qt_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) lib.resolve("glGenRenderbuffersEXT");
    qt_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) lib.resolve("glRenderbufferStorageEXT");
    qt_glGetRenderbufferParameterivEXT =
        (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) lib.resolve("glGetRenderbufferParameterivEXT");
    qt_glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC) lib.resolve("glIsFramebufferEXT");
    qt_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) lib.resolve("glBindFramebufferEXT");
    qt_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) lib.resolve("glDeleteFramebuffersEXT");
    qt_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) lib.resolve("glGenFramebuffersEXT");
    qt_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) lib.resolve("glCheckFramebufferStatusEXT");
    qt_glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) lib.resolve("glFramebufferTexture1DEXT");
    qt_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) lib.resolve("glFramebufferTexture2DEXT");
    qt_glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) lib.resolve("glFramebufferTexture3DEXT");
    qt_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) lib.resolve("glFramebufferRenderbufferEXT");
    qt_glGetFramebufferAttachmentParameterivEXT =
        (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) lib.resolve("glGetFramebufferAttachmentParameterivEXT");
    qt_glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) lib.resolve("glGenerateMipmapEXT");
    resolved = true;
    return qt_glIsRenderbufferEXT;
}
#elif defined(Q_WS_WIN)

#define QGL_FUNC_CONTEXT QGLContext *ctx = d_ptr->ctx;

#define glIsRenderbufferEXT ctx->d_ptr->qt_glIsRenderbufferEXT
#define glBindRenderbufferEXT ctx->d_ptr->qt_glBindRenderbufferEXT
#define glDeleteRenderbuffersEXT ctx->d_ptr->qt_glDeleteRenderbuffersEXT
#define glGenRenderbuffersEXT ctx->d_ptr->qt_glGenRenderbuffersEXT
#define glRenderbufferStorageEXT ctx->d_ptr->qt_glRenderbufferStorageEXT
#define glGetRenderbufferParameterivEXT ctx->d_ptr->qt_glGetRenderbufferParameterivEXT
#define glIsFramebufferEXT ctx->d_ptr->qt_glIsFramebufferEXT
#define glBindFramebufferEXT ctx->d_ptr->qt_glBindFramebufferEXT
#define glDeleteFramebuffersEXT ctx->d_ptr->qt_glDeleteFramebuffersEXT
#define glGenFramebuffersEXT ctx->d_ptr->qt_glGenFramebuffersEXT
#define glCheckFramebufferStatusEXT ctx->d_ptr->qt_glCheckFramebufferStatusEXT
#define glFramebufferTexture1DEXT ctx->d_ptr->qt_glFramebufferTexture1DEXT
#define glFramebufferTexture2DEXT ctx->d_ptr->qt_glFramebufferTexture2DEXT
#define glFramebufferTexture3DEXT ctx->d_ptr->qt_glFramebufferTexture3DEXT
#define glFramebufferRenderbufferEXT ctx->d_ptr->qt_glFramebufferRenderbufferEXT
#define glGetFramebufferAttachmentParameterivEXT ctx->d_ptr->qt_glGetFramebufferAttachmentParameterivEXT
#define glGenerateMipmapEXT ctx->d_ptr->qt_glGenerateMipmapEXT

bool qt_resolve_framebufferobject_extensions(QGLContext *ctx)
{
    if (glIsRenderbufferEXT != 0)
	return true;

    if (ctx == 0) {
	qWarning("QGLFramebufferObject: Unable to resolve framebuffer object extensions -"
		 " make sure there is a current context when creating the framebuffer object.");
	return false;
    }

    glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC) wglGetProcAddress("glIsRenderbufferEXT");
    glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) wglGetProcAddress("glBindRenderbufferEXT");
    glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) wglGetProcAddress("glDeleteRenderbuffersEXT");
    glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) wglGetProcAddress("glGenRenderbuffersEXT");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) wglGetProcAddress("glRenderbufferStorageEXT");
    glGetRenderbufferParameterivEXT =
        (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) wglGetProcAddress("glGetRenderbufferParameterivEXT");
    glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC) wglGetProcAddress("glIsFramebufferEXT");
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) wglGetProcAddress("glBindFramebufferEXT");
    glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) wglGetProcAddress("glDeleteFramebuffersEXT");
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) wglGetProcAddress("glGenFramebuffersEXT");
    glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
    glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) wglGetProcAddress("glFramebufferTexture1DEXT");
    glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
    glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) wglGetProcAddress("glFramebufferTexture3DEXT");
    glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
    glGetFramebufferAttachmentParameterivEXT =
        (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
    glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) wglGetProcAddress("glGenerateMipmapEXT");
    return glIsRenderbufferEXT;
}
#endif

#define QT_CHECK_GLERROR()                                \
{                                                         \
    GLenum err = glGetError();                            \
    if (err != GL_NO_ERROR) {                             \
        qDebug("[%s line %d] GL Error: %d",               \
               __FILE__, __LINE__, err);                  \
    }                                                     \
}

class QGLFramebufferObjectPrivate
{
public:
    QGLFramebufferObjectPrivate() : valid(false), ctx(0) {}
    ~QGLFramebufferObjectPrivate() {}

    void init(const QSize& sz, GLenum texture_target);
    bool checkFramebufferStatus() const;
    GLuint texture;
    GLuint fbo;
    GLuint depth_buffer;
    GLenum target;
    QSize size;
    uint valid : 1;
    QGLContext *ctx; // for Windows extension ptrs
};

bool QGLFramebufferObjectPrivate::checkFramebufferStatus() const
{
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status) {
    case GL_NO_ERROR:
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        return true;
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        qDebug("QGLFramebufferObject: Unsupported framebuffer format.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete attachment.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, missing attachment.");
        break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, duplicate attachment.");
        break;
#endif
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, attached images must have same dimensions.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, attached images must have same format.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, missing draw buffer.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, missing read buffer.");
        break;
    default:
        qDebug() <<"QGLFramebufferObject: An undefined error has occurred: "<< status;
        break;
    }
    return false;
}

void QGLFramebufferObjectPrivate::init(const QSize &sz, GLenum texture_target)
{
    ctx = const_cast<QGLContext *>(QGLContext::currentContext());
    bool ext_detected = (QGLExtensions::glExtensions & QGLExtensions::FramebufferObject);
    if (!ext_detected || (ext_detected && !qt_resolve_framebufferobject_extensions(ctx)))
        return;

    size = sz;
    target = texture_target;
    // texture dimensions

    glGetError(); // reset error state
    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

    QT_CHECK_GLERROR();
    // init texture
    glGenTextures(1, &texture);
    glBindTexture(target, texture);
#ifndef Q_WS_QWS
    glTexImage2D(target, 0, GL_RGBA8, size.width(), size.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
    glTexImage2D(target, 0, GL_RGBA, size.width(), size.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#endif
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              target, texture, 0);

    QT_CHECK_GLERROR();
    valid = checkFramebufferStatus();

#ifdef DEPTH_BUFFER
    // depth buffer
    glGenRenderbuffersEXT(1, &depth_buffer);
    Q_ASSERT(!glIsRenderbufferEXT(depth_buffer));
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_buffer);
    Q_ASSERT(glIsRenderbufferEXT(depth_buffer));
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, size.width(), size.height());
    int i = 0;
    glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &i);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                 GL_RENDERBUFFER_EXT, depth_buffer);

#endif
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    checkFramebufferStatus();
    QT_CHECK_GLERROR();
}

/*!
    \class QGLFramebufferObject
    \brief The QGLFramebufferObject class encapsulates an OpenGL framebuffer object.
    \since 4.2

    \ingroup multimedia

    The QGLFramebufferObject class encapsulates an OpenGL framebuffer
    object, defined by the \c{GL_EXT_framebuffer_object} extension. In
    addition it provides a rendering surface that can be painted on
    with a QPainter, rendered to using native GL calls, or both. This
    surface can be bound and used as a regular texture in your own GL
    drawing code.  By default, the QGLFramebufferObject class
    generates a 2D GL texture (using the \c{GL_TEXTURE_2D} target),
    which is used as the internal rendering target.

    \bold{It is important to have a current GL context when creating a
    QGLFramebufferObject, otherwise initialization will fail.}

    OpenGL framebuffer objects and pbuffers (see
    \l{QGLPixelBuffer}{QGLPixelBuffer}) can both be used to render to
    offscreen surfaces, but there are a number of advantages with
    using framebuffer objects instead of pbuffers:

    \list 1
    \o A framebuffer object does not require a separate rendering
    context, so no context switching will occur when switching
    rendering targets. There is an overhead involved in switching
    targets, but in general it is cheaper than a context switch to a
    pbuffer.

    \o Rendering to dynamic textures (i.e. render-to-texture
    functionality) works on all platforms. No need to do explicit copy
    calls from a render buffer into a texture, as was necessary on
    systems that did not support the \c{render_texture} extension.

    \o It is possible to attach several rendering buffers (or texture
    objects) to the same framebuffer object, and render to all of them
    without doing a context switch.

    \o The OpenGL framebuffer extension is a pure GL extension with no
    system dependant WGL, AGL or GLX parts. This makes using
    framebuffer objects more portable.
    \endlist

    Note that QPainter antialiasing of drawing primitives will not
    work when using a QGLFramebufferObject as a paintdevice. This is
    because sample buffers, which are needed for antialiasing, are not
    yet supported in application-defined framebuffer objects. However,
    an extension to solve this has already been approved by the OpenGL
    ARB (\c{GL_EXT_framebuffer_multisample}), and will most likely be
    available in the near future.

    \sa {Framebuffer Object Example}
*/


/*! \fn QGLFramebufferObject::QGLFramebufferObject(const QSize &size, GLenum target)

    Constructs an OpenGL framebuffer object and binds a 2D GL texture
    to the buffer of the size \a size. The texture is bound to the
    \c GL_COLOR_ATTACHMENT0 target in the framebuffer object.

    The \a target parameter is used to specify the GL texture
    target. The default target is \c GL_TEXTURE_2D. Keep in mind that
    \c GL_TEXTURE_2D textures must have a power of 2 width and height
    (e.g. 256x512).

    It is important that you have a current GL context set when
    creating the QGLFramebufferObject, otherwise the initialization
    will fail.

    \sa size(), texture()
*/
QGLFramebufferObject::QGLFramebufferObject(const QSize &size, GLenum target)
    : d_ptr(new QGLFramebufferObjectPrivate)
{
    Q_D(QGLFramebufferObject);
    d->init(size, target);
}


/*! \overload

    Constructs an OpenGL framebuffer object and binds a 2D GL texture
    to the buffer of the given \a width and \a height.

    \sa size(), texture()
*/
QGLFramebufferObject::QGLFramebufferObject(int width, int height, GLenum target)
    : d_ptr(new QGLFramebufferObjectPrivate)
{
    Q_D(QGLFramebufferObject);
    d->init(QSize(width, height), target);
}

/*!
    \fn QGLFramebufferObject::~QGLFramebufferObject()

    Destroys the framebuffer object and frees any allocated resources.
*/
QGLFramebufferObject::~QGLFramebufferObject()
{
    Q_D(QGLFramebufferObject);
    QGL_FUNC_CONTEXT;

    if (isValid()) {
	glDeleteTextures(1, &d->texture);
#ifdef DEPTH_BUFFER
	glDeleteRenderbuffersEXT(1, &d->depth_buffer);
#endif
	glDeleteFramebuffersEXT(1, &d->fbo);
    }
    delete d_ptr;
}

/*!
    \fn bool QGLFramebufferObject::isValid() const

    Returns true if the framebuffer object is valid.

    The framebuffer can become invalid if the initialization process
    fails, the user attaches an invalid buffer to the framebuffer
    object, or a non-power of 2 width/height is specified as the
    texture size if the texture target is \c{GL_TEXTURE_2D}.
*/
bool QGLFramebufferObject::isValid() const
{
    Q_D(const QGLFramebufferObject);
    return d->valid;
}

/*!
    \fn bool QGLFramebufferObject::bind()

    Switches rendering from the default, windowing system provided
    framebuffer to this framebuffer object.
    Returns true upon success, false otherwise.
*/
bool QGLFramebufferObject::bind()
{
    if (!isValid())
	return false;
    Q_D(QGLFramebufferObject);
    QGL_FUNC_CONTEXT;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, d->fbo);
    return d->checkFramebufferStatus();
}

/*!
    \fn bool QGLFramebufferObject::release()

    Switches rendering back to the default, windowing system provided
    framebuffer.
    Returns true upon success, false otherwise.
*/
bool QGLFramebufferObject::release()
{
    if (!isValid())
	return false;
    Q_D(QGLFramebufferObject);
    QGL_FUNC_CONTEXT;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    return d->checkFramebufferStatus();
}

/*!
    \fn GLuint QGLFramebufferObject::texture() const

    Returns the texture id for the texture attached as the default
    rendering target in this framebuffer object. This texture id can
    be bound as a normal texture in your own GL code.
*/
GLuint QGLFramebufferObject::texture() const
{
    Q_D(const QGLFramebufferObject);
    return d->texture;
}

/*!
    \fn QSize QGLFramebufferObject::size() const

    Returns the size of the texture attached to this framebuffer
    object.
*/
QSize QGLFramebufferObject::size() const
{
    Q_D(const QGLFramebufferObject);
    return d->size;
}

/*!
    \fn QImage QGLFramebufferObject::toImage() const

    Returns the contents of this framebuffer object as a QImage.
*/
QImage QGLFramebufferObject::toImage() const
{
    Q_D(const QGLFramebufferObject);
    if (!d->valid)
        return QImage();

    const_cast<QGLFramebufferObject *>(this)->bind();
    QImage::Format image_format = QImage::Format_RGB32;
    if (d->ctx->format().alpha())
        image_format = QImage::Format_ARGB32_Premultiplied;
    QImage img(d->size, image_format);
    int w = d->size.width();
    int h = d->size.height();
    // ### fix the read format so that we don't have to do all the byte swapping
    glReadPixels(0, 0, d->size.width(), d->size.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
	// OpenGL gives RGBA; Qt wants ARGB
	uint *p = (uint*)img.bits();
	uint *end = p + w*h;
	if (1) {
	    while (p < end) {
		uint a = *p << 24;
		*p = (*p >> 8) | a;
		p++;
	    }
	} else {
	    while (p < end) {
		*p = 0xFF000000 | (*p>>8);
		++p;
	    }
	}
    } else {
	// OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
	img = img.rgbSwapped();
    }
    const_cast<QGLFramebufferObject *>(this)->release();
    return img.mirrored();
}

Q_GLOBAL_STATIC(QOpenGLPaintEngine, qt_buffer_paintengine)

/*! \reimp */
QPaintEngine *QGLFramebufferObject::paintEngine() const
{
    return qt_buffer_paintengine();
}

/*!
    \fn bool QGLFramebufferObject::hasOpenGLFramebufferObjects()

    Returns true if the OpenGL \c{GL_EXT_framebuffer_object} extension
    is present on this system; otherwise returns false.
*/
bool QGLFramebufferObject::hasOpenGLFramebufferObjects()
{
    QGLWidget dmy; // needed to detect and init the QGLExtensions object
    return (QGLExtensions::glExtensions & QGLExtensions::FramebufferObject);
}

extern int qt_defaultDpi();

/*! \reimp */
int QGLFramebufferObject::metric(PaintDeviceMetric metric) const
{
    Q_D(const QGLFramebufferObject);

    float dpmx = qt_defaultDpi()*100./2.54;
    float dpmy = qt_defaultDpi()*100./2.54;
    int w = d->size.width();
    int h = d->size.height();
    switch (metric) {
    case PdmWidth:
        return w;

    case PdmHeight:
        return h;

    case PdmWidthMM:
        return qRound(w * 1000 / dpmx);

    case PdmHeightMM:
        return qRound(h * 1000 / dpmy);

    case PdmNumColors:
        return 0;

    case PdmDepth:
        return 32;//d->depth;

    case PdmDpiX:
        return (int)(dpmx * 0.0254);

    case PdmDpiY:
        return (int)(dpmy * 0.0254);

    case PdmPhysicalDpiX:
        return (int)(dpmx * 0.0254);

    case PdmPhysicalDpiY:
        return (int)(dpmy * 0.0254);

    default:
        qWarning("QGLFramebufferObject::metric(), Unhandled metric type: %d.\n", metric);
        break;
    }
    return 0;
}

/*!
    \fn GLuint QGLFramebufferObject::handle() const

    Returns the GL framebuffer object handle for this framebuffer
    object (returned by the \c{glGenFrameBuffersEXT()} function). This
    handle can be used to attach new images or buffers to the
    framebuffer. The user is responsible for cleaning up and
    destroying these objects.
*/
GLuint QGLFramebufferObject::handle() const
{
    Q_D(const QGLFramebufferObject);
    return d->fbo;
}

/*! \fn int QGLFramebufferObject::devType() const

    \reimp
*/
