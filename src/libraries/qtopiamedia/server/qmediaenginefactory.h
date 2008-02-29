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

#ifndef __QTOPIA_QMEDIAENGINEFACTORY_H
#define __QTOPIA_QMEDIAENGINEFACTORY_H


#include <QObject>

#include <qtopiaglobal.h>


class QMediaEngine;

class QTOPIAMEDIA_EXPORT QMediaEngineFactory
{
public:
    virtual ~QMediaEngineFactory() {}

    virtual QMediaEngine* create() = 0;
};

Q_DECLARE_INTERFACE(QMediaEngineFactory, "com.trolltech.qtopia.QMediaEngineFactory/1.0");


#endif  // __QTOPIA_QMEDIAENGINEFACTORY_H
