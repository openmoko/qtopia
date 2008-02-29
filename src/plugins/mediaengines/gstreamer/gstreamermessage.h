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

#ifndef __GSTREAMER_MESSAGE_H
#define __GSTREAMER_MESSAGE_H

#include <QMetaType>

#include <gst/gst.h>


namespace gstreamer
{

class Message
{
public:
    Message();
    Message(GstMessage* message);
    ~Message();

    GstMessage* rawMessage() const;

private:
    GstMessage* m_message;
};

}   // ns gstreamer

Q_DECLARE_METATYPE(gstreamer::Message);


#endif  // __GSTREAMER_MESSAGE_H

