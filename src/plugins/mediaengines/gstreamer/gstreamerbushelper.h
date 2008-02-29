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

#ifndef __GSTREAMER_BUSHELPER_H
#define __GSTREAMER_BUSHELPER_H

#include <QObject>

#include <gstreamermessage.h>

#include <gst/gst.h>


namespace gstreamer
{

class BusHelperPrivate;

class BusHelper : public QObject
{
    Q_OBJECT
    friend class BusHelperPrivate;

public:
    BusHelper(GstBus* bus, QObject* parent = 0);
    ~BusHelper();

signals:
    void message(Message message);

private:
    BusHelperPrivate*   d;
};

}   // ns gstreamer

#endif  // __GSTREAMER_BUSHELPER_H

