/****************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
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

#include "gstreamerx11painterwidget.h"
#include "gstreamerqtopiavideosink.h"

#include <QPainter>
#include <QX11Info>

namespace gstreamer {

X11PainterWidget::X11PainterWidget(QWidget* parent)
    : QWidget(parent)
    , m_sink(0)
{}

GstElement* X11PainterWidget::element()
{
    m_sink = GST_ELEMENT(g_object_new(QtopiaVideoSinkClass::get_type(), NULL));

    if (m_sink != 0) {
        QtopiaVideoSink*  sink = reinterpret_cast<QtopiaVideoSink*>(m_sink);
        sink->widget = this;
    }

    return m_sink;
}

int X11PainterWidget::displayDepth() const
{
    return QX11Info::appDepth();
}

void X11PainterWidget::setVideoSize(int, int)
{
    // FIXME, broken, GStreamer can scale, phonon
}

void X11PainterWidget::paint(const QImage& frame)
{
    m_image = frame;
    update();
}

void X11PainterWidget::paintEvent(QPaintEvent*)
{
    // FIXME, position it correctly..
    QPainter p(this);
    p.drawImage(0, 0, m_image);
}

}
