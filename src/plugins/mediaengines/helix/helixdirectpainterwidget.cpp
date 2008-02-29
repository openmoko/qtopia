/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <custom-qtopia.h>

#include "helixdirectpainterwidget.h"



static bool isQVFb()
{
    static bool result, test = true;

    if (test)
    {
        test = false;

        QByteArray display_spec(getenv("QWS_DISPLAY"));

        result = display_spec.startsWith("QVFb");
    }

    return result;
}



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
    HX_RELEASE(m_surface);
}

void DirectPainterVideoWidget::paintNotification()
{
    if (m_isVisible)
    {
        if (m_firstPaintCalc)
            calc(m_savedRegion);

        paint();
    }
}

int DirectPainterVideoWidget::isSupported()
{
    return true;
}

// private
void DirectPainterVideoWidget::regionChanged(const QRegion &exposedRegion)
{
    calc(exposedRegion);
}

void DirectPainterVideoWidget::calc(QRegion const& region)
{
    QImage const&   buffer = m_surface->buffer();

    m_isVisible = false;

    if (!region.isEmpty() && !buffer.isNull())
    {
        if (m_firstPaintCalc)   // XXX: Assume original exposed region is max widget size!
        {
            QRect bounds = region.boundingRect();
            QRect imageRect = buffer.rect();

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
            m_destSize = buffer.size();
            m_destSize.scale(bounds.size(), Qt::KeepAspectRatio);

            imageRect.setSize(m_destSize);
#endif
            imageRect.moveCenter(bounds.center());

            m_destRect = bounds;
            m_destTopLeft = imageRect.topLeft();
            m_blackRegion = region ^ QRegion(imageRect);

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

    QImage const& buffer = m_surface->buffer();

    if (!buffer.isNull() && !m_firstPaintCalc)
    {
        startPainting();

        QRegion paintRegion = allocatedRegion();

        if (m_black != 0)
        {
            qt_screen->solidFill(Qt::black, paintRegion & m_blackRegion);
            --m_black;
        }

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
        qt_screen->blit(buffer.scaled(m_destSize),
                        m_destTopLeft,
                        paintRegion);
#else
        qt_screen->blit(buffer, m_destTopLeft, paintRegion);
#endif
        endPainting();

        if (isQVFb())
        {
            // Inform QVFb of change in framebuffer
            QScreen::instance()->setDirty(m_destRect);
        }
    }

    m_painting = false;
}

