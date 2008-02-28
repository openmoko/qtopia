/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/qscreen_qws.h>
#include <QtGui/qwsevent_qws.h>
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/private/qwsdisplay_qws_p.h>
#include <QtGui/private/qwindowsurface_qws_p.h>

#include <custom.h>

#include "helixdirectpainterwidget.h"



DirectPainterVideoWidget::DirectPainterVideoWidget(GenericVideoSurface* surface, QWidget* parent):
    QDirectPainter(parent, QDirectPainter::NonReserved),
    m_surface(surface),
    m_isVisible(false),
    m_firstPaintCalc(true),
    m_painting(false),
    m_black(0)
{
    HX_ADDREF(m_surface);

    m_surface->addPaintObserver(this);
}

DirectPainterVideoWidget::~DirectPainterVideoWidget()
{
#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
    delete m_frameBufferImage;
#endif
    HX_RELEASE(m_surface);
}

void DirectPainterVideoWidget::paintNotification()
{
    if (m_isVisible) {
        if (m_firstPaintCalc) {
            calc(m_savedRegion);

        }
        paint();
    }
}

int DirectPainterVideoWidget::isSupported()
{
    return screenDepth() == 16 || screenDepth() == 32;
}

// private
void DirectPainterVideoWidget::regionChanged(const QRegion &exposedRegion)
{
    calc(exposedRegion);
}

static QRect nt;

void DirectPainterVideoWidget::calc(QRegion const& region)
{
    m_isVisible = false;

    QImage const& buffer = m_surface->buffer();

    if (!region.isEmpty() && !buffer.isNull()) {
        if (m_firstPaintCalc) {
            QRect bounds = geometry();
            m_destRect = buffer.rect();

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
            m_frameBufferImage = new QImage(frameBuffer(),
                                            screenWidth(),
                                            screenHeight(),
                                            screenDepth() == 16 ? QImage::Format_RGB16 :
                                                                  QImage::Format_RGB32);

            const double fourfifths = double(4) / 5;
            if ((m_destRect.width() < fourfifths * bounds.width() &&
                 m_destRect.height() < fourfifths * bounds.height()) ||
                (m_destRect.width() > bounds.width() ||
                 m_destRect.height() > bounds.height()))
            {
                QSize scaled = m_destRect.size();
                scaled.scale(bounds.size(), Qt::KeepAspectRatio);
                m_destRect.setSize(scaled);
            }

            m_destRect.moveCenter(bounds.center());

            if (qt_screen->isTransformed()) {
                m_windowRect = QRect(QPoint(0, 0), qt_screen->mapToDevice(buffer.size()));
                m_viewPort = qt_screen->mapToDevice(m_destRect, QSize(qt_screen->width(), qt_screen->height()));
                switch (qt_screen->transformOrientation()) {
                    case 1: m_transform.translate(0, m_windowRect.height()); break;
                    case 2: m_transform.translate(m_windowRect.width(), m_windowRect.height()); break;
                    case 3: m_transform.translate(m_windowRect.width(), 0); break;
                }
                m_transform.rotate(360 - qt_screen->transformOrientation() * 90);
            }
            else {
                m_windowRect = buffer.rect();
                m_viewPort = m_destRect;
            }
#else
            m_destRect.moveCenter(bounds.center());
            m_destTopLeft = m_destRect.topLeft();
#endif
            m_blackRegion = requestedRegion() ^ QRegion(m_destRect);

            m_firstPaintCalc = false;
        }
        else
            paint();        // force repaint in case paused

        // redraw black region (int(2) = weight against the race)
        m_black = m_blackRegion.isEmpty() ? 0 : 2;

        m_isVisible = true;
    }

    m_savedRegion = region;
}


void DirectPainterVideoWidget::paint()
{
    if (m_painting)
        return;

    m_painting = true;

    if (!m_firstPaintCalc) {
        QImage const& buffer = m_surface->buffer();

        startPainting();

        QRegion paintRegion = allocatedRegion();

        if (m_black != 0) {
            qt_screen->solidFill(Qt::black, paintRegion & m_blackRegion);
            --m_black;
        }

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
        QPainter    p(m_frameBufferImage);
        p.setClipRegion(paintRegion);
        p.setWindow(m_windowRect);
        p.setViewport(m_viewPort);
        p.setWorldTransform(m_transform);
        p.drawImage(0, 0, buffer);
#else
        qt_screen->blit(buffer, m_destTopLeft, paintRegion);
#endif
        qt_screen->setDirty(paintRegion.boundingRect());

        endPainting();
    }

    m_painting = false;
}

