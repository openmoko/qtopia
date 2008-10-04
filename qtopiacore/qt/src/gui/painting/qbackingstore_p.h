/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QBACKINGSTORE_P_H
#define QBACKINGSTORE_P_H

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

#include "private/qpaintengine_raster_p.h"
#include "private/qwidget_p.h"

QT_BEGIN_NAMESPACE

class QWindowSurface;

class Q_AUTOTEST_EXPORT QWidgetBackingStore
{
public:
    QWidgetBackingStore(QWidget *t);
    ~QWidgetBackingStore();
    bool bltRect(const QRect &rect, int dx, int dy, QWidget *widget);
    void dirtyRegion(const QRegion &rgn, QWidget *widget=0, bool updateImmediately = false);
#ifdef Q_RATE_LIMIT_PAINTING
    void updateDirtyRegion(QWidget *widget);
#endif
    void cleanRegion(const QRegion &rgn, QWidget *widget=0, bool recursiveCopyToScreen = true);
#if defined (Q_WS_QWS) || defined (Q_WS_WIN)
    void releaseBuffer();
#endif

    inline QPoint topLevelOffset() const { return tlwOffset; }
    static void copyToScreen(QWidget *, const QRegion &);
    static void qt_unflushPaint(QWidget *widget, const QRegion &rgn);
    static void qt_showYellowThing(QWidget *widget, const QRegion &rgn, int msec, bool);
#ifdef Q_WS_WIN
    static void blitToScreen(const QRegion &rgn, QWidget *w);
#endif
#ifdef Q_WIDGET_USE_DIRTYLIST
    void removeDirtyWidget(QWidget *w);
#endif

    static bool isOpaque(const QWidget *widget);
#ifdef Q_RATE_LIMIT_PAINTING
    static int refreshInterval;
#endif

private:
    QWidget *tlw;
#ifdef Q_WS_QWS
    QRegion dirtyOnScreen;
#else
    QRegion dirty;
#endif
#ifdef Q_WIDGET_USE_DIRTYLIST
    QList<QWidget*> dirtyWidgets;
#endif

    QWindowSurface *windowSurface;
#ifdef Q_BACKINGSTORE_SUBSURFACES
    QList<QWindowSurface*> subSurfaces;
#endif
    QPoint tlwOffset;

    void copyToScreen(const QRegion &rgn, QWidget *widget, const QPoint &offset, bool recursive = true);

    static void updateWidget(QWidget *that, const QRegion &rgn);

    friend void qt_syncBackingStore(QRegion, QWidget *);
#if defined(Q_WS_X11) || defined(Q_WS_QWS) || defined(Q_WS_WIN)
    friend void qt_syncBackingStore(QWidget *);
#endif
    friend QRegion qt_dirtyRegion(QWidget *, bool);
    friend class QWidgetPrivate;
    friend class QWidget;
    friend class QWSManagerPrivate;
    friend class QETWidget;
    friend class QWindowSurface;
    friend class QWSWindowSurface;
};

QT_END_NAMESPACE

#endif // QBACKINGSTORE_P_H
