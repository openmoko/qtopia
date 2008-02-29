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
#include <qglobal.h>
#include <qapplication.h>
#ifdef Q_WS_WIN
# include "qt_windows.h"
# include <private/qpaintengine_raster_p.h>
#endif
#include "qbackingstore_p.h"
#include "private/qwidget_p.h"
#include <qdebug.h>
#include <qstack.h>
#include <qevent.h>
#include <qabstractscrollarea.h>
#include "private/qabstractscrollarea_p.h"
#ifdef Q_WS_X11
# include "private/qt_x11_p.h"
#endif

#ifdef Q_WS_QWS
#include <qscreen_qws.h>
#include <qwsdisplay_qws.h>
#include <qapplication.h>
#include <qwsmanager_qws.h>
#include <private/qwsmanager_p.h>
#include <unistd.h>
#endif

#include "qwindowsurface_raster_p.h"
#ifdef Q_WS_X11
#include "qwindowsurface_x11_p.h"
#elif defined(Q_WS_QWS)
#include "qwindowsurface_qws_p.h"
#endif

/*****************************************************************************
  Top Level Window backing store
 *****************************************************************************/

extern bool qt_sendSpontaneousEvent(QObject*, QEvent*); // qapplication_xxx.cpp

#ifndef Q_WS_QWS
static bool qt_enable_backingstore = true;
#endif
#ifdef Q_WS_X11
// for compatibility with Qt 4.0
Q_GUI_EXPORT void qt_x11_set_global_double_buffer(bool enable)
{
    qt_enable_backingstore = enable;
}
#endif

bool QWidgetBackingStore::paintOnScreen(QWidget *w)
{
#if defined(Q_WS_QWS) || defined(Q_WS_MAC)
    Q_UNUSED(w);
    return false;
#elif  defined(QT_NO_BACKINGSTORE)
    Q_UNUSED(w);
    return true;
#else
    if (w && (w->testAttribute(Qt::WA_PaintOnScreen) || !w->isWindow() && w->window()->testAttribute(Qt::WA_PaintOnScreen)))
        return true;

    // sanity check for overlarge toplevels. Better: store at least screen size and move offset.
    if (w && w->isWindow() && (w->width() > 4096  || w->height() > 4096))
        return true;

    static signed char checked_env = -1;
    if(checked_env == -1)
        checked_env = (qgetenv("QT_ONSCREEN_PAINT") == "1") ? 1 : 0;

    return checked_env == 1 || !qt_enable_backingstore;
#endif
}

#ifdef Q_WS_QWS
static void qt_showYellowThing(QWidget *widget, const QRegion &rgn, int msec, bool)
{
    Q_UNUSED(widget);

    static QWSYellowSurface surface(true);
    surface.setDelay(msec);
    surface.flush(widget, rgn, QPoint());
}

#else
static void qt_showYellowThing(QWidget *widget, const QRegion &toBePainted, int msec, bool unclipped)
{
    //flags to fool painter
    bool paintUnclipped = widget->testAttribute(Qt::WA_PaintUnclipped);
    if (unclipped && !QWidgetBackingStore::paintOnScreen(widget))
        widget->setAttribute(Qt::WA_PaintUnclipped);

    bool setFlag = !widget->testAttribute(Qt::WA_WState_InPaintEvent);
    if(setFlag)
        widget->setAttribute(Qt::WA_WState_InPaintEvent);


    static int i = 0;

    //setup the engine
    QPaintEngine *pe = widget->paintEngine();
    if (pe) {
        pe->setSystemClip(toBePainted);
        {
            QPainter p(widget);
            p.setClipRegion(toBePainted);

            switch (i) {
            case 0:
                p.fillRect(widget->rect(), QColor(255,255,0));
                break;
            case 1:
                p.fillRect(widget->rect(), QColor(255,200,55));
                break;
            case 2:
                p.fillRect(widget->rect(), QColor(200,255,55));
                break;
            case 3:
                p.fillRect(widget->rect(), QColor(200,200,0));
                break;
            }
            i = (i+1) & 3;
            p.end();
        }
    }

    if(setFlag)
        widget->setAttribute(Qt::WA_WState_InPaintEvent, false);

    //restore
    widget->setAttribute(Qt::WA_PaintUnclipped, paintUnclipped);

    if (pe) {
        pe->setSystemClip(QRegion());
        //flush
        if (pe->type() == QPaintEngine::Raster) {
            QRasterPaintEngine *rpe = static_cast<QRasterPaintEngine *>(pe);
            rpe->flush(widget, QPoint());
        }
    }

    QApplication::syncX();

#if defined(Q_OS_UNIX)
    ::usleep(1000*msec);
#elif defined(Q_OS_WIN)
    ::Sleep(msec);
#endif

}
#endif

static bool qt_flushPaint(QWidget *widget, const QRegion &toBePainted)
{
    static int checked_env = -1;
    if(checked_env == -1)
        checked_env = qgetenv("QT_FLUSH_PAINT").toInt();

    if (checked_env == 0)
        return false;

    qt_showYellowThing(widget, toBePainted, checked_env*10, true);

    return true;
}

static void qt_unflushPaint(QWidget *widget, const QRegion &rgn)
{
    if (!QWidgetBackingStore::paintOnScreen(widget))
        QWidgetBackingStore::copyToScreen(widget, rgn);
}

#if !defined(Q_WS_QWS)
static bool qt_flushUpdate(QWidget *widget, const QRegion &rgn)
{
    static int checked_env = -1;
    if(checked_env == -1) {
        checked_env = qgetenv("QT_FLUSH_UPDATE").toInt();
    }

    if (checked_env == 0)
        return false;

    qt_showYellowThing(widget, rgn, checked_env*10, false);

    return true;
}
#endif

void qt_syncBackingStore(QRegion rgn, QWidget *widget, bool recursive)
{
    if (!QWidgetBackingStore::paintOnScreen(widget)) {
        if (QWidgetBackingStore *bs = widget->d_func()->maybeBackingStore())
            bs->cleanRegion(rgn, widget, recursive);
    } else {
        widget->repaint(rgn);
    }
}
void qt_syncBackingStore(QRegion rgn, QWidget *widget)
{
    qt_syncBackingStore(rgn, widget, false);
}

QWindowSurface *qt_default_window_surface(QWidget *widget)
{
#ifdef Q_WS_WIN
    return new QRasterWindowSurface(widget);
#elif defined(Q_WS_X11)
    return new QX11WindowSurface(widget);
#elif defined(Q_WS_QWS)
    if (widget->windowType() == Qt::Desktop)
        return 0;
    return qt_screen->createSurface(widget);
#else
    Q_UNUSED(widget);
    return 0;
#endif
}

#ifdef Q_WS_WIN

/*
   Used by QETWidget::translatePaintEvent and expects rgn to be in
   windowing system coordinates.
 */

void QWidgetBackingStore::blitToScreen(const QRegion &rgn, QWidget *widget)
{
    QWidget *tlw = widget->window();
    if (!widget->isVisible() || !tlw->testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;

    if (!QWidgetBackingStore::paintOnScreen(widget)) {
        QWidgetBackingStore *bs = tlw->d_func()->topData()->backingStore;

        bs->windowSurface->flush(widget, rgn, widget->mapTo(tlw, QPoint(0, 0)));
    }
}
#endif

#if defined(Q_WS_X11)
void qt_syncBackingStore(QWidget *widget)
{
    // dirtyOnScreen may get out of sync when widget is scrolled or moved
    widget->d_func()->dirtyOnScreen &= widget->d_func()->clipRect();

    const QRegion dirty =  widget->d_func()->dirtyOnScreen;
    QWidget *tlw = widget->window();
    if (!QWidgetBackingStore::paintOnScreen(widget)) {
        QWidgetBackingStore *bs = tlw->d_func()->topData()->backingStore;
        bs->cleanRegion(dirty, widget);
    } else {
        widget->repaint(dirty);
    }
}
#elif defined(Q_WS_QWS)
void qt_syncBackingStore(QWidget *widget)
{
    QWidget *tlw = widget->window();
    QTLWExtra *topData = tlw->d_func()->topData();

    QWidgetBackingStore *bs = topData->backingStore;
    QWSWindowSurface *surface = 0;
    if(bs)
        surface = static_cast<QWSWindowSurface*>(bs->windowSurface);
    else
        qWarning("request to sync backing store of widget %p, "
                 "which does not have its backing store defined yet",
                 (void*)widget);

    QRegion toClean;

    if (surface)
        toClean = surface->dirtyRegion();
    if (!toClean.isEmpty())
        topData->backingStore->cleanRegion(toClean, tlw);
}
#endif

/*
   A version of QRect::intersects() that does not normalize the rects.
*/
static inline bool qRectIntersects(const QRect &r1, const QRect &r2)
{
    return (qMax(r1.left(), r2.left()) <= qMin(r1.right(), r2.right()) &&
             qMax(r1.top(), r2.top()) <= qMin(r1.bottom(), r2.bottom()));
}

QWidgetBackingStore::QWidgetBackingStore(QWidget *t) : tlw(t)
{
    windowSurface = tlw->windowSurface();
    if (!windowSurface)
        windowSurface = qt_default_window_surface(t);
}

QWidgetBackingStore::~QWidgetBackingStore()
{
    if (!tlw->windowSurface())
        delete windowSurface;
}

/*
  Widget's coordinate system
  move whole rect by dx,dy
  rect must be valid
  doesn't generate any updates
*/
void QWidgetBackingStore::bltRect(const QRect &rect, int dx, int dy, QWidget *widget)
{
    QPoint pos(widget->mapTo(tlw, rect.topLeft()));

#ifdef Q_WS_QWS
    QWSWindowSurface *surface = static_cast<QWSWindowSurface*>(windowSurface);
    if (!surface)
        return;

    // clip to surface region
    const QRegion clip = surface->clipRegion();
    QRegion r = QRect(pos, rect.size());
    r = (r & clip).translated(dx, dy) & clip;
    r.translate(-dx, -dy);

    windowSurface->scroll(r.translated(topLevelOffset()), dx, dy);
#else
    windowSurface->scroll(QRect(pos, rect.size()), dx, dy);
#endif
}


//parent's coordinates; move whole rect; update parent and widget
//assume the screen blt has already been done, so we don't need to refresh that part
void QWidgetPrivate::moveRect(const QRect &rect, int dx, int dy)
{
    Q_Q(QWidget);
    if (!q->isVisible())
        return;

    QWidget *tlw = q->window();
    QTLWExtra* x = tlw->d_func()->topData();


    static int accelEnv = -1;
    if (accelEnv == -1) {
        accelEnv = qgetenv("QT_NO_FAST_MOVE").toInt() == 0;
    }

    QWidget *pw = q->parentWidget();
    QWidgetPrivate *pd = pw->d_func();
    QRect clipR = pd->clipRect();
    QRect newRect = rect.translated(dx,dy);

    QRect destRect = rect.intersected(clipR);
    if (destRect.isValid())
        destRect = destRect.translated(dx,dy).intersected(clipR);
    QRect sourceRect = destRect.translated(-dx, -dy);

    bool accelerateMove = accelEnv &&  isOpaque() && !isOverlapped(sourceRect)
        && !isOverlapped(destRect);

    if (!accelerateMove) {
        QRegion parentR(rect & clipR);
        if (q->mask().isEmpty()) {
            parentR -= newRect;
        } else {
            // invalidateBuffer() excludes anything outside the mask
            parentR += newRect & clipR;
        }
        pd->invalidateBuffer(parentR);
        invalidateBuffer((newRect & clipR).translated(-data.crect.topLeft()));
    } else {
        QWidgetBackingStore *wbs = x->backingStore;
        if (sourceRect.isValid())
            wbs->bltRect(sourceRect, dx, dy, pw);


        QRegion childExpose = newRect & clipR;
        childExpose -= destRect;

        QPoint toplevelOffset = pw->mapTo(tlw, QPoint());
        QRect newDirty = (wbs->dirty & sourceRect.translated(toplevelOffset)).boundingRect().translated(QPoint(dx,dy) - toplevelOffset);
        childExpose += newDirty;

        childExpose.translate(-data.crect.topLeft());
        invalidateBuffer(childExpose);

        QRegion parentExpose = rect & clipR;
        parentExpose -= newRect;
        if (!q->mask().isEmpty()) {
            parentExpose += QRegion(newRect) - q->mask().translated(data.crect.topLeft());
        }
        pd->invalidateBuffer(parentExpose);
#ifdef Q_WS_QWS
        //QWS does not have native child widgets: copy everything to screen, just like scrollRect()
        pd->dirtyWidget_sys(QRegion(sourceRect)+destRect);
#endif
    }
}

//widget's coordinates; scroll within rect;  only update widget
void QWidgetPrivate::scrollRect(const QRect &rect, int dx, int dy)
{
    Q_Q(QWidget);
    QWidget *tlw = q->window();
    QTLWExtra* x = tlw->d_func()->topData();
    QWidgetBackingStore *wbs = x->backingStore;

    static int accelEnv = -1;
    if (accelEnv == -1) {
        accelEnv = qgetenv("QT_NO_FAST_SCROLL").toInt() == 0;
    }

    bool accelerateScroll = accelEnv &&  isOpaque()  && !isOverlapped(data.crect);

#if defined(Q_WS_QWS)
    QWSWindowSurface *surface;
    surface = static_cast<QWSWindowSurface*>(wbs->windowSurface);

    if (accelerateScroll && !surface->isBuffered()) {
        const QRegion surfaceClip = surface->clipRegion();
        const QRegion outsideClip = QRegion(rect) - surfaceClip;
        if (!outsideClip.isEmpty()) {
            const QVector<QRect> clipped = (surfaceClip & rect).rects();
            if (clipped.size() < 8) {
                for (int i = 0; i < clipped.size(); ++i)
                    scrollRect(clipped.at(i), dx, dy);
                return;
            } else {
                accelerateScroll = false;
            }
        }
    }
#endif // Q_WS_QWS

    if (!accelerateScroll) {
        invalidateBuffer(rect);
    } else {
        QRect scrollRect = rect & clipRect();

        QRect destRect = scrollRect.isValid() ? scrollRect.translated(dx,dy).intersected(scrollRect) : QRect();
        QRect sourceRect = destRect.translated(-dx, -dy);

        QPoint toplevelOffset = q->mapTo(tlw, QPoint());


        if (sourceRect.isValid())
            wbs->bltRect(sourceRect, dx, dy, q);

        QRegion childExpose = scrollRect;
        childExpose -= destRect;
//        childExpose += (wbs->dirty & sourceRect.translated(toplevelOffset)).boundingRect().translated(QPoint(dx,dy) - toplevelOffset);
        QRect newDirty = (wbs->dirty & sourceRect.translated(toplevelOffset)).boundingRect().translated(QPoint(dx,dy) - toplevelOffset);
//         qDebug() << "scrollRect" << q << rect << dx << dy << "dirty" << wbs->dirty << "newDirty" << newDirty;
        childExpose += newDirty;
        invalidateBuffer(childExpose);

        // Instead of using native scroll-on-screen, we copy from
        // backingstore, giving only one screen update for each
        // scroll, and a solid appearance
        dirtyWidget_sys(rect);
    }
}

void QWidgetBackingStore::dirtyRegion(const QRegion &rgn, QWidget *widget)
{
    QRegion wrgn(rgn);
    Q_ASSERT(widget->window() == tlw);
    if(!widget->isVisible() || !widget->updatesEnabled())
        return;
    wrgn &= widget->d_func()->clipRect();
    if (!widget->mask().isEmpty())
        wrgn &= widget->mask();
#ifndef Q_WS_QWS
    widget->d_func()->dirtyWidget_sys(wrgn);
#endif
    wrgn.translate(widget->mapTo(tlw, QPoint(0, 0)));
    dirty += wrgn;
#ifdef Q_WS_QWS
    tlw->d_func()->dirtyWidget_sys(wrgn); //optimization: don't translate twice
#endif
}


void QWidgetBackingStore::copyToScreen(QWidget *widget, const QRegion &rgn)
{
    QWidget *tlw = widget->window();
    QTLWExtra *topextra = tlw->d_func()->extra->topextra;
    QPoint offset = widget->mapTo(tlw, QPoint());
    topextra->backingStore->copyToScreen(rgn, widget, offset, false);
}

/**
 * Copies the contents of the backingstore into the screen area of \a widget.
 * \a offset is the position of \a widget relative to the top level widget.
 * \a rgn is the region to be updated in \a widget coordinates.
 * \a recursive indicates that the widget should recursivly call copyToScreen
 * for all child widgets.
 */
void QWidgetBackingStore::copyToScreen(const QRegion &rgn, QWidget *widget, const QPoint &offset, bool recursive)
{
    if (rgn.isEmpty())
        return;
    Q_ASSERT(widget->testAttribute(Qt::WA_WState_Created));
#ifdef Q_WS_QWS
    Q_UNUSED(recursive);
    windowSurface->flush(widget, rgn, offset);
#else
    if (!QWidgetBackingStore::paintOnScreen(widget)) {
        widget->d_func()->cleanWidget_sys(rgn);

        qt_flushUpdate(widget, rgn);

        QPoint wOffset = widget->data->wrect.topLeft();
        windowSurface->flush(widget, rgn, offset);
    }

    if(recursive) {
        const QObjectList children = widget->children();
        for(int i = 0; i < children.size(); ++i) {
            if(QWidget *child = qobject_cast<QWidget*>(children.at(i))) {
                if(!child->isWindow() && child->isVisible()) {
                    if (qRectIntersects(rgn.boundingRect().translated(-child->pos()), child->rect())) {
                        QRegion childRegion(rgn);
                        childRegion.translate(-child->pos());
                        childRegion &= child->d_func()->clipRect();
                        if(!childRegion.isEmpty())
                            copyToScreen(childRegion, child, offset+child->pos(), recursive);
                    }
                }
            }
        }
    }
#endif
}

void QWidgetBackingStore::cleanRegion(const QRegion &rgn, QWidget *widget, bool recursiveCopyToScreen)
{
    if (!widget->isVisible() || !widget->updatesEnabled() || !tlw->testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;

    if(!QWidgetBackingStore::paintOnScreen(widget)) {
        QRegion toClean;
#if defined(Q_WS_QWS)
        QRect tlwRect = tlw->frameGeometry();
#else
        QRect tlwRect = tlw->geometry();
#endif

#ifdef Q_WS_QWS
        if (!static_cast<QWSWindowSurface*>(windowSurface)->isValidFor(tlw)) {
            delete windowSurface;
            windowSurface = qt_default_window_surface(tlw);
        }
#endif
        if (windowSurface->geometry() != tlwRect) {
            windowSurface->setGeometry(tlwRect);
            toClean = QRect(QPoint(0, 0), tlwRect.size());
            recursiveCopyToScreen = true;
        } else {
            toClean = dirty;
        }
#ifdef Q_WS_QWS
        tlwOffset = static_cast<QWSWindowSurface*>(windowSurface)->painterOffset();
#endif
         // ### move into prerender step

#if defined(Q_WS_QWS)
        const QRegion clip = static_cast<QWSWindowSurface*>(windowSurface)->clipRegion();
        if (!clip.isEmpty())
            toClean &= clip;
#endif

        if(!toClean.isEmpty()) {
            dirty -= toClean;
            if (tlw->updatesEnabled()) {
                // Pre render config
                windowSurface->paintDevice()->paintEngine()->setSystemClip(toClean);
                windowSurface->beginPaint(toClean);
                windowSurface->paintDevice()->paintEngine()->setSystemClip(QRegion());

                tlw->d_func()->drawWidget(windowSurface->paintDevice(), toClean, tlwOffset);

                // Drawing the overlay...
                windowSurface->paintDevice()->paintEngine()->setSystemClip(toClean);
                windowSurface->endPaint(toClean);
                windowSurface->paintDevice()->paintEngine()->setSystemClip(QRegion());
            }
        }

        QRegion toFlush = rgn;
        if (recursiveCopyToScreen) {
            toFlush.translate(widget->mapTo(tlw, QPoint()));
            copyToScreen(toFlush, tlw, tlwOffset, recursiveCopyToScreen);
        } else {
#ifdef Q_WS_X11
            toFlush += widget->d_func()->dirtyOnScreen;
#endif
            copyToScreen(toFlush, widget, widget->mapTo(tlw, QPoint()), false);
        }
    }
}

#ifdef Q_WS_QWS
void QWidgetBackingStore::releaseBuffer()
{
    if (windowSurface)
        windowSurface->release();
}
#elif defined(Q_WS_WIN)
void QWidgetBackingStore::releaseBuffer()
{
    windowSurface->release();
}
#endif

bool QWidgetBackingStore::isOpaque(const QWidget *widget)
{
    return widget->d_func()->isOpaque();
}


void QWidgetBackingStore::paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList& siblings, int index, const QRegion &rgn,
                                                 const QPoint &offset, int flags)
{
    QWidget *w = 0;

    do {
        QWidget *x =  qobject_cast<QWidget*>(siblings.at(index));
        if (x && !x->isWindow() && !x->isHidden() && qRectIntersects(rgn.boundingRect(), x->geometry())) {
            w = x;
            break;
        }
        --index;
    } while (index >= 0);


    if (!w)
        return;

    QWExtra *extra = w->d_func()->extraData();

    if (index > 0) {
        QRegion wr = rgn;
        if (isOpaque(w)) {
            if(!extra || extra->mask.isEmpty()) {
                wr -= w->geometry();
            } else {
                wr -= extra->mask.translated(w->pos());
            }
        }
        paintSiblingsRecursive(pdev, siblings, index - 1, wr, offset, flags);
    }
    if(w->updatesEnabled()) {
        QRegion wRegion(rgn & w->geometry());
        wRegion.translate(-w->pos());

        if(extra && !extra->mask.isEmpty())
            wRegion &= extra->mask;
        if(!wRegion.isEmpty())
            w->d_func()->drawWidget(pdev, wRegion, offset+w->pos(), flags);
    }
}

void QWidgetPrivate::drawWidget(QPaintDevice *pdev, const QRegion &rgn, const QPoint &offset, int flags)
{
    Q_Q(QWidget);
    if (rgn.isEmpty())
        return;

    const bool asRoot = flags & DrawAsRoot;
    const bool alsoOnScreen = flags & DrawPaintOnScreen;
    const bool recursive = flags & DrawRecursive;
    const bool alsoInvisible = flags & DrawInvisible;

    QRegion toBePainted = rgn;
    if (asRoot && !alsoInvisible)
        toBePainted &= clipRect(); //(rgn & visibleRegion());
    subtractOpaqueChildren(toBePainted, q->rect(), QPoint());

    if (!toBePainted.isEmpty()) {
        bool onScreen = QWidgetBackingStore::paintOnScreen(q);
        if (!onScreen || alsoOnScreen) {
            //update the "in paint event" flag
            if (q->testAttribute(Qt::WA_WState_InPaintEvent))
                qWarning("QWidget::repaint: Recursive repaint detected");
            q->setAttribute(Qt::WA_WState_InPaintEvent);

            //clip away the new area
            bool flushed = qt_flushPaint(q, toBePainted);

            QPaintEngine *paintEngine = pdev->paintEngine();
            if (paintEngine) {
                QPainter::setRedirected(q, pdev, -offset);

                QRegion wrgn = toBePainted;
                wrgn.translate(offset);
                paintEngine->setSystemRect(q->data->crect);
                paintEngine->setSystemClip(wrgn);

                //paint the background
                if ((asRoot || q->autoFillBackground() || onScreen)
                    && !q->testAttribute(Qt::WA_OpaquePaintEvent)
                    && !q->testAttribute(Qt::WA_NoSystemBackground)) {

                    QPainter p(q);
                    QRect backgroundRect = toBePainted.boundingRect();

#ifndef QT_NO_SCROLLAREA
                    if (QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(q->parent())) {
                        if (scrollArea->viewport() == q) {
                            QAbstractScrollAreaPrivate *priv = static_cast<QAbstractScrollAreaPrivate *>(static_cast<QWidget *>(scrollArea)->d_ptr);
                            const QPoint offset = priv->contentsOffset();
                            p.translate(-offset);
                            backgroundRect.translate(offset);
                        }
                    }
#endif // QT_NO_SCROLLAREA
                    paintBackground(&p, backgroundRect, asRoot || onScreen);
                }
                if (q->testAttribute(Qt::WA_TintedBackground)
                    && !onScreen && !asRoot && !isOpaque() ) {
                    QPainter p(q);
                    QColor tint = q->palette().window().color();
                    tint.setAlphaF(.6);
                    p.fillRect(toBePainted.boundingRect(), tint);
                }
            }

#if 0
            qDebug() << "painting" << q << "opaque ==" << isOpaque();
            qDebug() << "clipping to" << toBePainted << "location == " << offset
                     << "geometry ==" << QRect(q->mapTo(q->window(), QPoint(0, 0)), q->size());
#endif

            //actually send the paint event
            QPaintEvent e(toBePainted);
            qt_sendSpontaneousEvent(q, &e);

            //restore
            if (paintEngine) {
                paintEngine->setSystemRect(QRect());
                pdev->paintEngine()->setSystemClip(QRegion());
                QPainter::restoreRedirected(q);
            }
            q->setAttribute(Qt::WA_WState_InPaintEvent, false);
            if(!q->testAttribute(Qt::WA_PaintOutsidePaintEvent) && q->paintingActive())
                qWarning("QWidget::repaint: It is dangerous to leave painters active on a widget outside of the PaintEvent");

            if (flushed)
                qt_unflushPaint(q, toBePainted);
        } else if(q->isWindow()) {
            if (pdev->paintEngine()) {
                QPainter p(pdev);
                p.setClipRegion(toBePainted);
                const QBrush bg = q->palette().brush(QPalette::Window);
                if (bg.style() == Qt::TexturePattern)
                    p.drawTiledPixmap(q->rect(), bg.texture());
                else
                    p.fillRect(q->rect(), bg);
            }
        }
    }

    if (recursive) {
        const QObjectList children = q->children();
        if (!children.isEmpty())
            QWidgetBackingStore::paintSiblingsRecursive(pdev, children, children.size()-1, rgn, offset, flags & ~DrawAsRoot);
    }
}

/* cross-platform QWidget code */

void QWidgetPrivate::invalidateBuffer(const QRegion &rgn)
{
    if(qApp && qApp->closingDown())
        return;
    Q_Q(QWidget);
    if (QWidgetBackingStore *bs = maybeBackingStore())
        bs->dirtyRegion(rgn, q);
}

void QWidget::repaint(const QRegion& rgn)
{
    if (testAttribute(Qt::WA_WState_ConfigPending)) {
        update(rgn);
        return;
    }

    if (!isVisible() || !updatesEnabled() || rgn.isEmpty())
        return;
    Q_D(QWidget);
    Q_ASSERT(testAttribute(Qt::WA_WState_Created));
//    qDebug() << "repaint" << this << rgn;
    if (!QWidgetBackingStore::paintOnScreen(this)) {
        if (QWidgetBackingStore *bs = d->maybeBackingStore()) {
            QRegion wrgn(rgn);
            d->subtractOpaqueSiblings(wrgn, QPoint());
            d->subtractOpaqueChildren(wrgn, rect(), QPoint());
            bs->dirtyRegion(wrgn, this);
            bs->cleanRegion(wrgn, this);
        }
    }
#ifndef Q_WS_QWS
// QWS doesn't support paint-on-screen
    else {
        d->cleanWidget_sys(rgn);
        //     qDebug() << "QWidget::repaint paintOnScreen" << this << "region" << rgn;
      qt_flushPaint(this, rgn);

        QPaintEngine *engine = paintEngine();

        QRegion systemClipRgn(rgn);

        if (engine) {
            if (!data->wrect.topLeft().isNull()) {
                QPainter::setRedirected(this, this, data->wrect.topLeft());
                systemClipRgn.translate(-data->wrect.topLeft());
            }
            engine->setSystemClip(systemClipRgn);
            engine->setSystemRect(data->crect);
        }

        d->drawWidget(this, rgn, QPoint(), QWidgetPrivate::DrawAsRoot | QWidgetPrivate::DrawPaintOnScreen);

#ifdef Q_WS_WIN
        if (engine && engine->type() == QPaintEngine::Raster) {
            bool tmp_dc = !d->hd;
            if (tmp_dc)
                d->hd = GetDC(winId());
            static_cast<QRasterPaintEngine *>(engine)->flush(this, QPoint(0, 0));
            if (tmp_dc) {
                ReleaseDC(winId(), (HDC)d->hd);
                d->hd = 0;
            }
        }
#endif
        if (engine) {
            if (!data->wrect.topLeft().isNull())
                QPainter::restoreRedirected(this);
            engine->setSystemClip(QRegion());
            engine->setSystemRect(QRect());
        }

        if(!testAttribute(Qt::WA_PaintOutsidePaintEvent) && paintingActive())
            qWarning("QWidget::repaint: It is dangerous to leave painters active on a widget outside of the PaintEvent");
    }
#endif //Q_WS_QWS
}

void QWidget::update()
{
    update(rect());
}

void QWidget::update(const QRect &r)
{
    update(QRegion(r));
}

void QWidget::update(const QRegion& rgn)
{
    if(!isVisible() || !updatesEnabled() || rgn.isEmpty())
        return;

    Q_D(QWidget);
    if (testAttribute(Qt::WA_WState_InPaintEvent)) {
        QApplication::postEvent(this, new QUpdateLaterEvent(rgn));
    } else {
        if (QWidgetBackingStore *bs = d->maybeBackingStore()) {
            QRegion wrgn(rgn);
            d->subtractOpaqueSiblings(wrgn, QPoint());
            d->subtractOpaqueChildren(wrgn, rect(), QPoint());
            bs->dirtyRegion(wrgn, this);
        }
    }
}

