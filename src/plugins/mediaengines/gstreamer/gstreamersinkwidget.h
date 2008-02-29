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

#ifndef __GSTREAMER_SINKWIDGET_H
#define __GSTREAMER_SINKWIDGET_H


#include <QWidget>

#include <gst/video/gstvideosink.h>


namespace gstreamer
{

class SinkWidget
{
public:
    virtual ~SinkWidget();

    virtual GstElement* element() = 0;

    virtual int displayDepth() const = 0;
    virtual void setVideoSize(int width, int height) = 0;
    virtual void paint(QImage const& frame) = 0;
};

}   // ns gstreamer

#endif  // __GSTREAMER_SINKWIDGET_H

