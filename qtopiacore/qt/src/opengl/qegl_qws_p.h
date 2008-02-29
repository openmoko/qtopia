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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QEGL.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QEGL_H
#define QEGL_H

#include <EGL/egl.h>


class QImage;
class QRect;

class /*Q_OPENGL_EXPORT*/ QEGL
{
public:
    static NativeWindowType createNativeWindow(const QRect&);
    //static NativeWindowType toNativeWindow(QWidget *);
    static NativePixmapType createNativePixmap(QImage *);
    static void destroyNativePixmap(NativePixmapType);
    // static NativePixmapType toNativePixmap(QPixmap *);
};


#endif
