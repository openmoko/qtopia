/****************************************************************************
**
** Copyright (C) 2007-2007 TROLLTECH ASA. All rights reserved.
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

#include "examplescreen.h"
#include <QRect>
#include <QRegion>

#include <QDebug>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


ExampleScreen::ExampleScreen(int displayId)
: QLinuxFbScreen(displayId)
{
    fbd = ::open( "/dev/fb0", O_RDWR );
    if ( fbd < 0 ) {
        qWarning() << "ExampleScreen: Cannot open frame buffer device /dev/fb0";
        return;
    }
}

ExampleScreen::~ExampleScreen()
{
}

void ExampleScreen::exposeRegion(QRegion region, int changing)
{
    QLinuxFbScreen::exposeRegion( region, changing );
    dirtyrect = region.boundingRect();
    //add your own flush here to update the dirty rect!!
}

