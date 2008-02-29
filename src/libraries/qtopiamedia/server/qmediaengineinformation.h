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

#ifndef __QTOPIA_MEDIAENGINEINFORMATION_H
#define __QTOPIA_MEDIAENGINEINFORMATION_H


#include <QString>
#include <QList>
#include <QMediaSessionBuilder>

#include <qtopiaglobal.h>


class QTOPIAMEDIA_EXPORT QMediaEngineInformation
{
public:
    virtual ~QMediaEngineInformation();

    virtual QString name() const = 0;
    virtual QString version() const = 0;

    virtual int idleTime() const = 0;

    virtual bool hasExclusiveDeviceAccess() const = 0;

    virtual QMediaSessionBuilderList sessionBuilders() const = 0;
};


#endif  // __QTOPIA_MEDIAENGINEINFORMATION_H
