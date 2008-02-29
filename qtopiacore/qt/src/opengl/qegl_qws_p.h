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
