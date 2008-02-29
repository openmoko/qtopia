/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGL_P_H
#define QGL_P_H

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

#include "QtOpenGL/qgl.h"
#include "QtOpenGL/qglcolormap.h"
#include "QtCore/qmap.h"
#include "QtCore/qthread.h"
#include "QtCore/qthreadstorage.h"
#include "QtCore/qhash.h"
#include "private/qwidget_p.h"

class QGLContext;
class QGLOverlayWidget;
class QPixmap;
#ifdef Q_WS_MAC
#include <AGL/agl.h>
class QMacWindowChangeEvent;
#endif

#ifdef Q_WS_QWS
#include <GLES/egl.h>
class QGLDirectPainter;
#endif

// extension prototypes
#ifndef Q_WS_MAC
# ifndef APIENTRYP
#   ifdef APIENTRY
#     define APIENTRYP APIENTRY *
#   else
#     define APIENTRY
#     define APIENTRYP *
#   endif
# endif
#else
# define APIENTRY
# define APIENTRYP *
#endif

// ARB_fragment_program
typedef void (APIENTRY *_glProgramStringARB) (GLenum, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY *_glBindProgramARB) (GLenum, GLuint);
typedef void (APIENTRY *_glDeleteProgramsARB) (GLsizei, const GLuint *);
typedef void (APIENTRY *_glGenProgramsARB) (GLsizei, GLuint *);
typedef void (APIENTRY *_glProgramLocalParameter4fvARB) (GLenum, GLuint, const GLfloat *);

// GLSL
typedef GLuint (APIENTRY *_glCreateShader) (GLenum);
typedef void (APIENTRY *_glShaderSource) (GLuint, GLsizei, const char **, const GLint *);
typedef void (APIENTRY *_glCompileShader) (GLuint);
typedef void (APIENTRY *_glDeleteShader) (GLuint);

typedef GLuint (APIENTRY *_glCreateProgram) ();
typedef void (APIENTRY *_glAttachShader) (GLuint, GLuint);
typedef void (APIENTRY *_glDetachShader) (GLuint, GLuint);
typedef void (APIENTRY *_glLinkProgram) (GLuint);
typedef void (APIENTRY *_glUseProgram) (GLuint);
typedef void (APIENTRY *_glDeleteProgram) (GLuint);

typedef void (APIENTRY *_glGetShaderInfoLog) (GLuint, GLsizei, GLsizei *, char *);
typedef void (APIENTRY *_glGetProgramiv) (GLuint, GLenum, GLint *);

typedef GLuint (APIENTRY *_glGetUniformLocation) (GLuint, const char*);
typedef void (APIENTRY *_glUniform4fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform3fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform2fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform1fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform1i) (GLint, GLint);

// EXT_GL_framebuffer_object
typedef GLboolean (APIENTRYP PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget,
                                                           GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget,
                                                           GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget,
                                                           GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget,
                                                              GLuint renderbuffer);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname,
                                                                          GLint *params);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);

class QGLFormatPrivate
{
public:
    QGLFormatPrivate() {
        opts = QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba | QGL::DirectRendering | QGL::StencilBuffer;
        pln = 0;
        depthSize = accumSize = stencilSize = redSize = greenSize = blueSize = alphaSize = -1;
        numSamples = -1;
        swapInterval = -1;
    }
    QGL::FormatOptions opts;
    int pln;
    int depthSize;
    int accumSize;
    int stencilSize;
    int redSize;
    int greenSize;
    int blueSize;
    int alphaSize;
    int numSamples;
    int swapInterval;
};

class QGLWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGLWidget)
public:
    QGLWidgetPrivate() : QWidgetPrivate() {}
    ~QGLWidgetPrivate() {}

    void init(QGLContext *context, const QGLWidget* shareWidget);
    void initContext(QGLContext *context, const QGLWidget* shareWidget);
    bool renderCxPm(QPixmap *pixmap);
    void cleanupColormaps();

    QGLContext *glcx;
    bool autoSwap;

    QGLColormap cmap;
    QMap<QString, int> displayListCache;

#if defined(Q_WS_WIN)
    void updateColormap();
    QGLContext *olcx;
#elif defined(Q_WS_X11)
    QGLOverlayWidget *olw;
#elif defined(Q_WS_MAC)
    QGLContext *olcx;
    void updatePaintDevice();
    QMacWindowChangeEvent *watcher;
#elif defined(Q_WS_QWS)
    QGLDirectPainter *directPainter;
    void resizeHandler(const QSize &);
    void render(const QRegion&);
#endif
};

class QGLContextPrivate
{
    Q_DECLARE_PUBLIC(QGLContext)
public:
    explicit QGLContextPrivate(QGLContext *context) : q_ptr(context)
    {
#ifdef Q_WS_WIN
        qt_glProgramStringARB = 0;
        qt_glBindProgramARB = 0;
        qt_glDeleteProgramsARB = 0;
        qt_glGenProgramsARB = 0;
        qt_glProgramLocalParameter4fvARB = 0;

        qt_glCreateShader = 0;
        qt_glShaderSource = 0;
        qt_glCompileShader = 0;
        qt_glDeleteShader = 0;

        qt_glCreateProgram = 0;
        qt_glAttachShader = 0;
        qt_glDetachShader = 0;
        qt_glLinkProgram = 0;
        qt_glUseProgram = 0;
        qt_glDeleteProgram = 0;

        qt_glGetShaderInfoLog = 0;
        qt_glGetProgramiv = 0;

        qt_glGetUniformLocation = 0;
        qt_glUniform4fv = 0;
        qt_glUniform3fv = 0;
        qt_glUniform2fv = 0;
        qt_glUniform1fv = 0;
        qt_glUniform1i = 0;

        qt_glIsRenderbufferEXT = 0;
        qt_glBindRenderbufferEXT = 0;
        qt_glDeleteRenderbuffersEXT = 0;
        qt_glGenRenderbuffersEXT = 0;
        qt_glRenderbufferStorageEXT = 0;
        qt_glGetRenderbufferParameterivEXT = 0;
        qt_glIsFramebufferEXT = 0;
        qt_glBindFramebufferEXT = 0;
        qt_glDeleteFramebuffersEXT = 0;
        qt_glGenFramebuffersEXT = 0;
        qt_glCheckFramebufferStatusEXT = 0;
        qt_glFramebufferTexture1DEXT = 0;
        qt_glFramebufferTexture2DEXT = 0;
        qt_glFramebufferTexture3DEXT = 0;
        qt_glFramebufferRenderbufferEXT = 0;
        qt_glGetFramebufferAttachmentParameterivEXT = 0;
        qt_glGenerateMipmapEXT = 0;
#endif
    }
    ~QGLContextPrivate() {}
    GLuint bindTexture(const QImage &image, GLenum target, GLint format, const QString &key,
                       qint64 qt_id, bool clean = false);
    GLuint bindTexture(const QPixmap &pixmap, GLenum target, GLint format, bool clean);
    GLuint bindTexture(const QImage &image, GLenum target, GLint format, bool clean);
    bool textureCacheLookup(const QString &key, GLuint *id, qint64 *qt_id);
    void init(QPaintDevice *dev, const QGLFormat &format);
    QImage convertToBGRA(const QImage &image, bool force_premul);

#if defined(Q_WS_WIN)
    HGLRC rc;
    HDC dc;
    WId        win;
    int pixelFormatId;
    QGLCmap* cmap;
    HBITMAP hbitmap;
    HDC hbitmap_hdc;
#elif defined(Q_WS_X11) || defined(Q_WS_MAC)
    void* vi;
    void* cx;
#if defined(Q_WS_X11)
    quint32 gpm;
#endif
#if defined(Q_WS_MAC)
    bool update;
    AGLPixelFormat tryFormat(const QGLFormat &format);
#endif
#elif defined(Q_WS_QWS)
    EGLDisplay dpy;
    EGLContext cx;
    EGLConfig  config;
    EGLSurface surface;
#endif
    QGLFormat glFormat;
    QGLFormat reqFormat;

    uint valid : 1;
    uint sharing : 1;
    uint initDone : 1;
    uint crWin : 1;
    QPaintDevice *paintDevice;
    QColor transpColor;
    QGLContext *q_ptr;

#ifdef Q_WS_WIN
    _glProgramStringARB qt_glProgramStringARB;
    _glBindProgramARB qt_glBindProgramARB;
    _glDeleteProgramsARB qt_glDeleteProgramsARB;
    _glGenProgramsARB qt_glGenProgramsARB;
    _glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB;

    // GLSL definitions
    _glCreateShader qt_glCreateShader;
    _glShaderSource qt_glShaderSource;
    _glCompileShader qt_glCompileShader;
    _glDeleteShader qt_glDeleteShader;

    _glCreateProgram qt_glCreateProgram;
    _glAttachShader qt_glAttachShader;
    _glDetachShader qt_glDetachShader;
    _glLinkProgram qt_glLinkProgram;
    _glUseProgram qt_glUseProgram;
    _glDeleteProgram qt_glDeleteProgram;

    _glGetShaderInfoLog qt_glGetShaderInfoLog;
    _glGetProgramiv qt_glGetProgramiv;

    _glGetUniformLocation qt_glGetUniformLocation;
    _glUniform4fv qt_glUniform4fv;
    _glUniform3fv qt_glUniform3fv;
    _glUniform2fv qt_glUniform2fv;
    _glUniform1fv qt_glUniform1fv;
    _glUniform1i qt_glUniform1i;

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
#endif // Q_WS_WIN
};

// GL extension definitions
class QGLExtensions {
public:
    enum Extension {
        TextureRectangle        = 0x00000001,
        SampleBuffers           = 0x00000002,
        GenerateMipmap          = 0x00000004,
        TextureCompression      = 0x00000008,
        FragmentProgram         = 0x00000010,
        MirroredRepeat          = 0x00000020,
        FramebufferObject       = 0x00000040
    };
    Q_DECLARE_FLAGS(Extensions, Extension)

    static Extensions glExtensions;
    static void init(); // sys dependent
    static void init_extensions(); // general: called by init()
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGLExtensions::Extensions)

/* NV_texture_rectangle */
#ifndef GL_NV_texture_rectangle
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV   0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV     0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV  0x84F8
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

struct QGLThreadContext {
    QGLContext *context;
};
extern QThreadStorage<QGLThreadContext *> qgl_context_storage;

typedef QMultiHash<const QGLContext *, const QGLContext *> QGLSharingHash;
class QGLShareRegister
{
public:
    QGLShareRegister() {}
    ~QGLShareRegister() { reg.clear(); }

    bool checkSharing(const QGLContext *context1, const QGLContext *context2, const QGLContext * skip=0) {
        QList<const QGLContext *> shares = reg.values(context1);
        for (int k=0; k<shares.size(); ++k) {
            const QGLContext *ctx = shares.at(k);
            if (ctx == skip) // avoid an indirect circular loop (infinite recursion)
                continue;
            if (ctx == context2)
                return true;
            if (checkSharing(ctx, context2, context1))
                return true;
        }
        return false;
    }

    void addShare(const QGLContext *context, const QGLContext *share) {
        reg.insert(context, share); // context sharing works both ways
        reg.insert(share, context);
    }

    void removeShare(const QGLContext *context) {
        QGLSharingHash::iterator it = reg.begin();
        while (it != reg.end()) {
            if (it.key() == context || it.value() == context)
                it = reg.erase(it);
            else
                ++it;
        }
    }

private:
    QGLSharingHash reg;
};

extern QGLShareRegister* qgl_share_reg();
#endif // QGL_P_H
