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

#ifndef GSTREAMERX11PAINTERWIDGET_H
#define GSTREAMERX11PAINTERWIDGET_H

#include "gstreamersinkwidget.h"

namespace gstreamer
{

/*
 * Mostly a copy of DirectPainterWidget. The code that could
 * be shared would go into a common base class.
 * GStreamer has a XVWidget and more research would need to happen
 * to findout how to embed this into the Qtopia context, but this is
 * kind of void because Qt 4.4 has Phonon which has a GStreamer backend
 * that can use XV... so create something that compiles and works for
 * video but will not be a speed killer.
 */
class X11PainterWidget : public QWidget, public SinkWidget {
    Q_OBJECT

public:
    X11PainterWidget(QWidget* parent = 0);

    GstElement* element();

    int displayDepth() const;
    void setVideoSize(int width, int height);
    void paint(QImage const& frame);

protected:
    void paintEvent(QPaintEvent*);

private:
    GstElement* m_sink;
    QImage m_image;
};

}

#endif
