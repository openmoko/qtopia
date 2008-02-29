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

#ifndef __GSTREAMER_DIRECTPAINTERSINK_H
#define __GSTREAMER_DIRECTPAINTERSINK_H

#include <QDirectPainter>

#include "gstreamersinkwidget.h"


namespace gstreamer
{

class DirectPainterWidget :
    public QDirectPainter,
    public SinkWidget
{
    Q_OBJECT

public:
    DirectPainterWidget(QWidget* parent = 0);
    ~DirectPainterWidget();

    // Sink widget
    GstElement* element();

    int displayDepth() const;
    void setVideoSize(int width, int height);
    void paint(QImage const& frame);

private:
    void regionChanged(const QRegion &exposedRegion);
    void calc(QRegion const& region);

    bool        m_isVisible;
    bool        m_firstPaintCalc;
    bool        m_painting;
    int         m_black;
    QPoint      m_destTopLeft;
    QSize       m_destSize;
    QSize       m_videoSize;
    QRect       m_destRect;
    QRegion     m_savedRegion;
    QRegion     m_blackRegion;
    GstElement* m_sink;
};

}   // ns gstreamer


#endif  // __GSTREAMER_DIRECTPAINTERSINK_H
