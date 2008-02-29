/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QPIXMAP_P_H
#define QPIXMAP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qpixmap.h"
#if defined(Q_WS_X11)
#include "QtGui/qx11info_x11.h"
#endif


#if defined(Q_WS_WIN) || defined(Q_WS_QWS)

struct QPixmapData { // internal pixmap data
    QPixmapData() : count(1) { }
    ~QPixmapData(){};
    void ref() { ++count; }
    bool deref() { return !--count; }
    int count;
    int detach_no;
    QImage image;
    QPixmap::Type type;

    QImage createBitmapImage(int w, int h);
};

#else // non raster

struct QPixmapData { // internal pixmap data
    QPixmapData() : count(1) { }
    ~QPixmapData();

    void ref() { ++count; }
    bool deref() { return !--count; }
    int count;
    QPixmap::Type type;

    int w, h;
    short d;
    uint uninit:1;
    int ser_no;
    int detach_no;
#if !defined(Q_WS_X11) && !defined(Q_WS_MAC)
    QBitmap *mask;
#endif
#if defined(Q_WS_X11)
    QX11Info xinfo;
    Qt::HANDLE x11_mask;
    Qt::HANDLE picture;
    Qt::HANDLE mask_picture;
    Qt::HANDLE hd2; // sorted in the default display depth
    Qt::HANDLE x11ConvertToDefaultDepth();
#ifndef QT_NO_XRENDER
    void convertToARGB32();
#endif
#elif defined(Q_WS_MAC)
    uint has_alpha : 1, has_mask : 1;
    void macSetHasAlpha(bool b);
    void macGetAlphaChannel(QPixmap *, bool asMask) const;
    void macSetAlphaChannel(const QPixmap *, bool asMask);
    void macQDDisposeAlpha();
    void macQDUpdateAlpha();
    quint32 *pixels;
    uint nbytes;
    QRectF cg_mask_rect;
    CGImageRef cg_data, cg_mask;
    GWorldPtr qd_data, qd_alpha;
#endif
    QPaintEngine *paintEngine;
#if !defined(Q_WS_MAC)
    Qt::HANDLE hd;
#endif
#ifdef Q_WS_X11
    QBitmap mask_to_bitmap(int screen) const;
    static Qt::HANDLE bitmap_to_mask(const QBitmap &, int screen);
#endif
    static int allocCell(const QPixmap *p);
    static void freeCell(QPixmapData *data, bool terminate = false);
};

#endif // Q_WS_WIN

#  define QT_XFORM_TYPE_MSBFIRST 0
#  define QT_XFORM_TYPE_LSBFIRST 1
#  if defined(Q_WS_WIN)
#    define QT_XFORM_TYPE_WINDOWSPIXMAP 2
#  endif
extern bool qt_xForm_helper(const QMatrix&, int, int, int, uchar*, int, int, int, const uchar*, int, int, int);

#endif // QPIXMAP_P_H
