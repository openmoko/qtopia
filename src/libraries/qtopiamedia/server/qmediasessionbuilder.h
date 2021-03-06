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

#ifndef __QTOPIA_MEDIA_QMEDIASESSIONBUILDER_H
#define __QTOPIA_MEDIA_QMEDIASESSIONBUILDER_H


#include <QVariant>
#include <QList>
#include <QMap>

#include <qtopiaglobal.h>


class QMediaServerSession;
class QMediaSessionRequest;

class QTOPIAMEDIA_EXPORT QMediaSessionBuilder
{
public:
    typedef QMap<QString, QVariant> Attributes;

    virtual ~QMediaSessionBuilder();

    virtual QString type() const = 0;
    virtual Attributes const& attributes() const = 0;

    virtual QMediaServerSession* createSession(QMediaSessionRequest sessionRequest) = 0;
    virtual void destroySession(QMediaServerSession* serverSession) = 0;
};

typedef QList<QMediaSessionBuilder*> QMediaSessionBuilderList;

#endif  // __QTOPIA_MEDIA_QMEDIASESSIONBUILDER_H

