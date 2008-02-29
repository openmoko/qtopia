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

#include "helixgenericvideowidget.h"


GenericVideoWidget::GenericVideoWidget(GenericVideoSurface* surface, QWidget* parent):
    QWidget(parent),
    m_surface(surface)
{
    HX_ADDREF( m_surface );

    m_surface->addPaintObserver(this);

    // Optimize paint event
    setAttribute(Qt::WA_NoSystemBackground);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    QPalette pal(palette());
    pal.setBrush(QPalette::Window, Qt::black);

    setPalette(pal);
}

GenericVideoWidget::~GenericVideoWidget()
{
    HX_RELEASE(m_surface);
}

void GenericVideoWidget::paintNotification()
{
    QWidget::update();
}

void GenericVideoWidget::paintEvent( QPaintEvent* )
{
    QPainter        painter(this);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImage const& buffer = m_surface->buffer();

    if (!buffer.isNull())
    {
        QSize scaled = buffer.size();

        scaled.scale(width(), height(), Qt::KeepAspectRatio);

        painter.drawImage(QRect(QPoint((width() - scaled.width()) / 2, (height() - scaled.height()) / 2 ), scaled),
                          buffer,
                          buffer.rect());
    }
}


