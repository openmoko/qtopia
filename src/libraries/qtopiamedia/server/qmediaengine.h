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

#ifndef __QTOPIA_QMEDIAENGINE_H
#define __QTOPIA_QMEDIAENGINE_H

#include <QObject>
#include <QList>

#include <qtopiaglobal.h>


class QMediaEngineInformation;

class QTOPIAMEDIA_EXPORT QMediaEngine : public QObject
{
    Q_OBJECT

public:
    virtual ~QMediaEngine();

    virtual void initialize() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void suspend() = 0;
    virtual void resume() = 0;

    virtual QMediaEngineInformation const* engineInformation() = 0;
};

typedef QList<QMediaEngine*>    QMediaEngineList;


#endif  //  __QTOPIA_MEDIAENGINE_H

