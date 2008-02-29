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

#ifndef __GSTREAMER_VIDEOSINK_H
#define __GSTREAMER_VIDEOSINK_H


#include <gst/video/gstvideosink.h>


namespace gstreamer
{

class SinkWidget;

class QtopiaVideoSink
{
public:
    GstVideoSink    videoSink;

    SinkWidget*     widget;
    gint            width;
    gint            height;
    gint            bpp;
    gint            depth;

    static GstCaps* get_caps(GstBaseSink* sink);
    static gboolean set_caps(GstBaseSink* sink, GstCaps* caps);
    static GstStateChangeReturn change_state(GstElement* element, GstStateChange transition);
    static GstFlowReturn render(GstBaseSink* sink, GstBuffer* buf);
    static void base_init(gpointer g_class);
    static void instance_init(GTypeInstance *instance, gpointer g_class);
};

struct QtopiaVideoSinkClass
{
    GstVideoSinkClass   parent_class;

    static void class_init(gpointer g_class, gpointer class_data);
    static GType get_type();
};

}   // ns gstreamer


#endif  // __GSTREAMER_VIDEOSINK_H
