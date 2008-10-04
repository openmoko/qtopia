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
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
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


#ifdef QT_OPENGL_ES_CL

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QGLWidget class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

inline void glTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
    glTexParameterx(target, pname, param);
}
inline void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    glClearColorx(FLOAT2X(red) ,FLOAT2X(green), FLOAT2X(blue), FLOAT2X(alpha));
}
inline void glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    glColor4x(FLOAT2X(red) ,FLOAT2X(green), FLOAT2X(blue), FLOAT2X(alpha));
}

inline void glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    glOrthox(FLOAT2X(left), FLOAT2X(right), FLOAT2X(bottom), FLOAT2X(top), FLOAT2X(zNear), FLOAT2X(zFar));
}

inline  void glPointSize (GLfloat size)
{
    glPointSizex(FLOAT2X(size));
}

inline void glPolygonOffset (GLfloat factor, GLfloat units)
{
    glPolygonOffsetx (FLOAT2X(factor), FLOAT2X(units));
}

inline void glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    glRotatex(FLOAT2X(angle), FLOAT2X(x), FLOAT2X(y), FLOAT2X(z));
}

inline void glTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
    glTranslatex(FLOAT2X(x) ,FLOAT2X(y) ,FLOAT2X(z));
}

inline void glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz)
{
    glNormal3x(FLOAT2X(nx), FLOAT2X(ny), FLOAT2X(nz));
}

inline void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    glScalex(FLOAT2X(x), FLOAT2X(y), FLOAT2X(z));
}

inline void glClearDepthf (GLclampf depth)
{
    glClearDepthx(FLOAT2X(depth));
}

inline void glAlphaFunc (GLenum func, GLclampf ref)
{
    glAlphaFuncx(func, FLOAT2X(ref));
}

inline void glLoadMatrixf (const GLfloat *_m)
{
    GLfixed m[16];
    for (int i =0; i < 16; i++)
        m[i] = FLOAT2X(_m[i]);
    glLoadMatrixx(m);
}

inline void glMultMatrixf (const GLfloat *_m)
{
    GLfixed m[16];
    for (int i =0; i < 16; i++)
        m[i] = FLOAT2X(_m[i]);
    glMultMatrixx (m);
}


inline void glLineWidth (GLfloat width)
{
    glLineWidthx(FLOAT2X(width));
}

QT_END_NAMESPACE

#endif  //QT_OPENGL_ES_CL

