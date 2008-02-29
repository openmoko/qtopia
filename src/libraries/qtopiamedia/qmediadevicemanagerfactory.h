/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef __QTOPIA_MEDIALIBRARY_MEDIADEVICEMANAGERFACTORY_H
#define __QTOPIA_MEDIALIBRARY_MEDIADEVICEMANAGERFACTORY_H

#include <qobject.h>


class QMediaDeviceManager;


/*!
    \class QMediaDeviceManagerFactory
    \internal
*/

class QMediaDeviceManagerFactory
{
public:
    virtual ~QMediaDeviceManagerFactory() {}

    virtual QMediaDeviceManager* create() = 0;
};


Q_DECLARE_INTERFACE(QMediaDeviceManagerFactory, "com.trolltech.qtopia.MediaDeviceManagerFactory/1.0");

#endif  // __QTOPIA_MEDIALIBRARY_MEDIADEVICEMANAGERFACTORY_H
