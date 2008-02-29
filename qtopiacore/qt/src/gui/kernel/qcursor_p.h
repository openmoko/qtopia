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

#ifndef QCURSOR_P_H
#define QCURSOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qatomic.h"
#include "QtCore/qglobal.h"
#include "QtCore/qnamespace.h"
#include "QtGui/qpixmap.h"

# if defined (Q_WS_MAC)
#  include "private/qt_mac_p.h"
#  if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
#    define QMAC_USE_BIG_CURSOR_API
#  endif
#  ifndef QMAC_NO_FAKECURSOR
     class QMacCursorWidget;
#  endif
   class QMacAnimateCursor;
# elif defined(Q_WS_X11)
#  include "private/qt_x11_p.h"
# elif defined(Q_WS_WIN)
#  include "QtCore/qt_windows.h"
#endif

class QBitmap;
struct QCursorData {
    QCursorData(Qt::CursorShape s = Qt::ArrowCursor);
    ~QCursorData();

    static void initialize();
    static void cleanup();

    QAtomic ref;
    Qt::CursorShape cshape;
    QBitmap  *bm, *bmm;
    QPixmap pixmap;
    short     hx, hy;
#if defined (Q_WS_MAC) || defined(Q_WS_QWS)
    int id;
#endif
#if defined (Q_WS_WIN)
    HCURSOR hcurs;
#elif defined (Q_WS_X11)
    XColor fg, bg;
    Cursor hcurs;
    Pixmap pm, pmm;
#elif defined (Q_WS_MAC)
    enum { TYPE_None, TYPE_CursPtr, TYPE_ThemeCursor, TYPE_FakeCursor, TYPE_BigCursor } type;
    union {
        struct {
            uint my_cursor:1;
            CursPtr   hcurs;
        } cp;
#ifndef QMAC_NO_FAKECURSOR
        struct {
            QMacCursorWidget *widget;
            CursPtr empty_curs;
        } fc;
#endif
#ifdef QMAC_USE_BIG_CURSOR_API
        char *big_cursor_name;
#endif
        struct {
            QMacAnimateCursor *anim;
            ThemeCursor curs;
        } tc;
    } curs;
#endif
    static bool initialized;
    void update();
    static QCursorData *setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY);
};

#endif // QCURSOR_P_H
