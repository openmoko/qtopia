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

#include "cruxusoutputthread.h"

#include "cruxusoutputdevices.h"


namespace cruxus
{


int OutputDevices::created = 0;
OutputThread* OutputDevices::output = 0;

/*!
    \class cruxus::OutputDevices
    \internal
*/

QMediaDevice* OutputDevices::createOutputDevice()
{
    if (output == 0)
        output = new OutputThread;

    created++;

    return output;
}

void OutputDevices::destroyOutputDevice(QMediaDevice* )
{
    if (--created == 0)
    {
        delete output;
        output = 0;
    }
}

}   // ns cruxus
