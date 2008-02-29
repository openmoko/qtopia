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


#ifndef __CRUXUX_OUTPUTDEVICES_H
#define __CRUXUX_OUTPUTDEVICES_H

class QMediaDevice;

namespace cruxus
{

class OutputThread;

class OutputDevices
{
    static int              created;
    static OutputThread*    output;

public:
    static QMediaDevice* createOutputDevice();
    static void destroyOutputDevice(QMediaDevice* outputDevice);
};

}   // ns cruxus

#endif  // __CRUXUX_OUTPUTDEVICES_H
